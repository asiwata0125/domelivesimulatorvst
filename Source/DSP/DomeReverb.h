/*
  ==============================================================================
    DomeReverb.h
    ドームライブリバーブ - 東京ドームのようなアリーナ音響をエミュレート
    
    改善版: 真のステレオリバーブ、低域強化、プリセット対応
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "CombFilter.h"
#include "AllPassFilter.h"
#include <array>
#include <algorithm>

// プリセット列挙型
enum class DomePreset
{
    Arena,    // 東京ドーム風（デフォルト）
    Stadium,  // 野外スタジアム風
    Hall,     // コンサートホール風
    Club      // ライブハウス風
};

class DomeReverb
{
public:
    DomeReverb() = default;
    ~DomeReverb() = default;

    // サンプルレートとブロックサイズで初期化
    void prepare(double newSampleRate, int samplesPerBlock)
    {
        sampleRate = newSampleRate;

        // 左チャンネル用遅延時間（素数で設定すると金属音を避けられる）
        const float combDelaysL[8] = { 
            29.7f, 37.1f, 41.1f, 43.7f,
            47.3f, 53.9f, 59.3f, 61.7f
        };
        
        // 右チャンネル用遅延時間（左より少し長くしてステレオ感を出す）
        const float combDelaysR[8] = { 
            31.1f, 39.7f, 43.3f, 47.1f,
            51.7f, 57.3f, 63.1f, 67.9f
        };

        const float allPassDelaysL[4] = { 5.0f, 6.7f, 10.0f, 12.4f };
        const float allPassDelaysR[4] = { 5.3f, 7.1f, 11.3f, 13.7f };

        // 左チャンネルのコムフィルターを初期化
        for (int i = 0; i < 8; ++i)
        {
            combFiltersL[i].prepare(sampleRate, 150.0f);
            combFiltersL[i].setDelayTime(combDelaysL[i]);
            combFiltersL[i].setFeedback(0.82f);
            combFiltersL[i].setDamping(0.3f);
        }

        // 右チャンネルのコムフィルターを初期化
        for (int i = 0; i < 8; ++i)
        {
            combFiltersR[i].prepare(sampleRate, 150.0f);
            combFiltersR[i].setDelayTime(combDelaysR[i]);
            combFiltersR[i].setFeedback(0.82f);
            combFiltersR[i].setDamping(0.3f);
        }

        // 左チャンネルのオールパスフィルターを初期化
        for (int i = 0; i < 4; ++i)
        {
            allPassFiltersL[i].prepare(sampleRate, 30.0f);
            allPassFiltersL[i].setDelayTime(allPassDelaysL[i]);
            allPassFiltersL[i].setCoefficient(0.5f);
        }

        // 右チャンネルのオールパスフィルターを初期化
        for (int i = 0; i < 4; ++i)
        {
            allPassFiltersR[i].prepare(sampleRate, 30.0f);
            allPassFiltersR[i].setDelayTime(allPassDelaysR[i]);
            allPassFiltersR[i].setCoefficient(0.5f);
        }

        // プリディレイ（短縮: 最大30ms）
        int maxPreDelaySamples = static_cast<int>(50.0f * sampleRate / 1000.0f);
        preDelayBufferL.resize(maxPreDelaySamples, 0.0f);
        preDelayBufferR.resize(maxPreDelaySamples, 0.0f);
        preDelayWriteIndexL = 0;
        preDelayWriteIndexR = 0;

        // ローパスフィルター（L/R独立）
        lowPassFilterL.setCoefficients(
            juce::IIRCoefficients::makeLowPass(sampleRate, 8000.0)
        );
        lowPassFilterR.setCoefficients(
            juce::IIRCoefficients::makeLowPass(sampleRate, 8000.0)
        );

        // ローシェルフフィルター（低域強化）
        lowShelfFilterL.setCoefficients(
            juce::IIRCoefficients::makeLowShelf(sampleRate, 200.0, 0.7f, 1.5f)
        );
        lowShelfFilterR.setCoefficients(
            juce::IIRCoefficients::makeLowShelf(sampleRate, 200.0, 0.7f, 1.5f)
        );

        // ==========================================================
        // プリEQ（リバーブ前のEQカーブ）- FL Studio画像に基づく
        // ==========================================================
        
        // バンド1（紫/50Hz）: わずかに持ち上げ +1dB
        preEQ_Band1L.setCoefficients(
            juce::IIRCoefficients::makeLowShelf(sampleRate, 50.0, 0.7, 1.12f) // +1dB
        );
        preEQ_Band1R.setCoefficients(
            juce::IIRCoefficients::makeLowShelf(sampleRate, 50.0, 0.7, 1.12f)
        );

        // バンド2（ピンク/100Hz）: 少し下げ -1dB
        preEQ_Band2L.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 100.0, 1.5, 0.89f) // -1dB
        );
        preEQ_Band2R.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 100.0, 1.5, 0.89f)
        );

        // バンド3（オレンジ/200Hz）: ディップ -3dB
        preEQ_Band3L.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 200.0, 1.0, 0.71f) // -3dB
        );
        preEQ_Band3R.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 200.0, 1.0, 0.71f)
        );

        // バンド4（イエロー/400Hz）: 最も深いカット -4dB
        preEQ_Band4L.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 400.0, 1.2, 0.63f) // -4dB
        );
        preEQ_Band4R.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 400.0, 1.2, 0.63f)
        );

        // バンド5（緑/1kHz）: 少し持ち上げ +2dB
        preEQ_Band5L.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 1000.0, 1.0, 1.26f) // +2dB
        );
        preEQ_Band5R.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 1000.0, 1.0, 1.26f)
        );

        // バンド6（水色/4kHz）: 大きなピーク +6dB
        preEQ_Band6L.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 4000.0, 1.5, 2.0f) // +6dB
        );
        preEQ_Band6R.setCoefficients(
            juce::IIRCoefficients::makePeakFilter(sampleRate, 4000.0, 1.5, 2.0f)
        );

        // バンド7（青/10kHz〜）: 急激なローパス
        preEQ_Band7L.setCoefficients(
            juce::IIRCoefficients::makeLowPass(sampleRate, 10000.0, 0.5)
        );
        preEQ_Band7R.setCoefficients(
            juce::IIRCoefficients::makeLowPass(sampleRate, 10000.0, 0.5)
        );
    }

    // ドーム感の量を設定（0.0 - 1.0）
    void setDomeAmount(float amount)
    {
        domeAmount = std::clamp(amount, 0.0f, 1.0f);
        updateParameters();
    }

    float getDomeAmount() const { return domeAmount; }

    // プリセットを適用
    void setPreset(DomePreset preset)
    {
        currentPreset = preset;
        switch (preset)
        {
            case DomePreset::Arena:
                domeAmount = 0.6f;
                stereoWidth = 0.8f;
                bassBoost = 1.5f;
                break;
            case DomePreset::Stadium:
                domeAmount = 0.8f;
                stereoWidth = 1.0f;
                bassBoost = 1.8f;
                break;
            case DomePreset::Hall:
                domeAmount = 0.4f;
                stereoWidth = 0.6f;
                bassBoost = 1.2f;
                break;
            case DomePreset::Club:
                domeAmount = 0.25f;
                stereoWidth = 0.5f;
                bassBoost = 2.0f;
                break;
        }
        updateParameters();
    }

    DomePreset getPreset() const { return currentPreset; }

    // オーディオバッファを処理
    void process(juce::AudioBuffer<float>& buffer)
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (numChannels == 0) return;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // ステレオ入力を取得
            float inputL = buffer.getSample(0, sample);
            float inputR = numChannels > 1 ? buffer.getSample(1, sample) : inputL;

            // ==========================================================
            // プリEQを適用（リバーブに送る前のEQカーブ）
            // ==========================================================
            float eqL = inputL;
            float eqR = inputR;

            // バンド1（50Hz）: わずかに持ち上げ
            eqL = preEQ_Band1L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band1R.processSingleSampleRaw(eqR);

            // バンド2（100Hz）: 少し下げ
            eqL = preEQ_Band2L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band2R.processSingleSampleRaw(eqR);

            // バンド3（200Hz）: ディップ
            eqL = preEQ_Band3L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band3R.processSingleSampleRaw(eqR);

            // バンド4（400Hz）: 最も深いカット
            eqL = preEQ_Band4L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band4R.processSingleSampleRaw(eqR);

            // バンド5（1kHz）: 少し持ち上げ
            eqL = preEQ_Band5L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band5R.processSingleSampleRaw(eqR);

            // バンド6（4kHz）: 大きなピーク
            eqL = preEQ_Band6L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band6R.processSingleSampleRaw(eqR);

            // バンド7（10kHz〜）: ローパス
            eqL = preEQ_Band7L.processSingleSampleRaw(eqL);
            eqR = preEQ_Band7R.processSingleSampleRaw(eqR);

            // プリディレイを適用（L/R独立）- EQ処理済みの信号を使用
            float preDelayedL = processPreDelay(eqL, preDelayBufferL, preDelayWriteIndexL, preDelaySamplesL);
            float preDelayedR = processPreDelay(eqR, preDelayBufferR, preDelayWriteIndexR, preDelaySamplesR);

            // L/R独立したコムフィルターを通す
            float combOutL = 0.0f;
            float combOutR = 0.0f;
            for (int i = 0; i < 8; ++i)
            {
                combOutL += combFiltersL[i].process(preDelayedL);
                combOutR += combFiltersR[i].process(preDelayedR);
            }
            combOutL *= 0.125f;  // 1/8
            combOutR *= 0.125f;

            // クロスフィード（ステレオイメージを自然にする）
            float crossFeedAmount = 0.15f;
            float tempL = combOutL + combOutR * crossFeedAmount;
            float tempR = combOutR + combOutL * crossFeedAmount;
            combOutL = tempL;
            combOutR = tempR;

            // L/R独立したオールパスフィルターで拡散
            for (int i = 0; i < 4; ++i)
            {
                combOutL = allPassFiltersL[i].process(combOutL);
                combOutR = allPassFiltersR[i].process(combOutR);
            }

            // ローパスフィルター（高域を減衰）
            float filteredL = lowPassFilterL.processSingleSampleRaw(combOutL);
            float filteredR = lowPassFilterR.processSingleSampleRaw(combOutR);

            // ローシェルフフィルター（低域強化）
            filteredL = lowShelfFilterL.processSingleSampleRaw(filteredL);
            filteredR = lowShelfFilterR.processSingleSampleRaw(filteredR);

            // ステレオ幅を適用
            float mid = (filteredL + filteredR) * 0.5f;
            float side = (filteredL - filteredR) * 0.5f * stereoWidth;
            filteredL = mid + side;
            filteredR = mid - side;

            // Wet/Dry ミックス
            float wetL = filteredL * wetGain;
            float wetR = filteredR * wetGain;
            float dryL = inputL * dryGain;
            float dryR = inputR * dryGain;

            buffer.setSample(0, sample, dryL + wetL);
            if (numChannels > 1)
                buffer.setSample(1, sample, dryR + wetR);
        }
    }

    // バッファをクリア
    void clear()
    {
        for (auto& comb : combFiltersL)
            comb.clear();
        for (auto& comb : combFiltersR)
            comb.clear();
        for (auto& ap : allPassFiltersL)
            ap.clear();
        for (auto& ap : allPassFiltersR)
            ap.clear();
        std::fill(preDelayBufferL.begin(), preDelayBufferL.end(), 0.0f);
        std::fill(preDelayBufferR.begin(), preDelayBufferR.end(), 0.0f);
        lowPassFilterL.reset();
        lowPassFilterR.reset();
        lowShelfFilterL.reset();
        lowShelfFilterR.reset();
    }

private:
    // ワンノブに基づいてパラメータを更新
    void updateParameters()
    {
        // Wet/Dry ミックス（ノブが上がるほどWetが増える）
        wetGain = domeAmount * 0.6f;  // 最大60%のWet（控えめに）
        dryGain = 1.0f - (domeAmount * 0.3f);  // 最低70%のDry

        // プリディレイ（短縮版: 最大30ms、L/Rで少しずらす）
        preDelaySamplesL = static_cast<int>(domeAmount * 25.0f * sampleRate / 1000.0f);
        preDelaySamplesR = static_cast<int>(domeAmount * 30.0f * sampleRate / 1000.0f);
        
        if (preDelaySamplesL >= static_cast<int>(preDelayBufferL.size()))
            preDelaySamplesL = static_cast<int>(preDelayBufferL.size()) - 1;
        if (preDelaySamplesR >= static_cast<int>(preDelayBufferR.size()))
            preDelaySamplesR = static_cast<int>(preDelayBufferR.size()) - 1;

        // フィードバック（ノブが上がるほどRT60が長く）
        float feedback = 0.75f + domeAmount * 0.12f; // 0.75 - 0.87
        for (auto& comb : combFiltersL)
            comb.setFeedback(feedback);
        for (auto& comb : combFiltersR)
            comb.setFeedback(feedback);

        // ダンピング（ノブが上がるほど高域が減衰）
        float damping = 0.15f + domeAmount * 0.35f; // 0.15 - 0.5
        for (auto& comb : combFiltersL)
            comb.setDamping(damping);
        for (auto& comb : combFiltersR)
            comb.setDamping(damping);

        // ローパスカットオフ
        float cutoff = 10000.0f - domeAmount * 5000.0f; // 10kHz - 5kHz
        lowPassFilterL.setCoefficients(
            juce::IIRCoefficients::makeLowPass(sampleRate, cutoff)
        );
        lowPassFilterR.setCoefficients(
            juce::IIRCoefficients::makeLowPass(sampleRate, cutoff)
        );

        // ローシェルフ（低域ブースト）
        lowShelfFilterL.setCoefficients(
            juce::IIRCoefficients::makeLowShelf(sampleRate, 200.0, 0.7f, bassBoost)
        );
        lowShelfFilterR.setCoefficients(
            juce::IIRCoefficients::makeLowShelf(sampleRate, 200.0, 0.7f, bassBoost)
        );
    }

    // プリディレイ処理（L/R独立）
    float processPreDelay(float input, std::vector<float>& buffer, int& writeIndex, int delaySamples)
    {
        if (delaySamples == 0)
            return input;

        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0)
            readIndex += static_cast<int>(buffer.size());

        float delayed = buffer[readIndex];
        buffer[writeIndex] = input;

        writeIndex++;
        if (writeIndex >= static_cast<int>(buffer.size()))
            writeIndex = 0;

        return delayed;
    }

    double sampleRate = 44100.0;
    float domeAmount = 0.5f;
    float stereoWidth = 0.8f;
    float bassBoost = 1.5f;
    DomePreset currentPreset = DomePreset::Arena;

    // エフェクトパラメータ
    float wetGain = 0.3f;
    float dryGain = 0.85f;

    // DSPコンポーネント（L/R独立）
    std::array<CombFilter, 8> combFiltersL;
    std::array<CombFilter, 8> combFiltersR;
    std::array<AllPassFilter, 4> allPassFiltersL;
    std::array<AllPassFilter, 4> allPassFiltersR;

    // プリディレイ（L/R独立）
    std::vector<float> preDelayBufferL;
    std::vector<float> preDelayBufferR;
    int preDelayWriteIndexL = 0;
    int preDelayWriteIndexR = 0;
    int preDelaySamplesL = 0;
    int preDelaySamplesR = 0;

    // フィルター（L/R独立）
    juce::IIRFilter lowPassFilterL;
    juce::IIRFilter lowPassFilterR;
    juce::IIRFilter lowShelfFilterL;
    juce::IIRFilter lowShelfFilterR;

    // プリEQフィルター（リバーブ前のEQカーブ）- FL Studio画像に基づく
    juce::IIRFilter preEQ_Band1L;   // 50Hz ローシェルフ +1dB
    juce::IIRFilter preEQ_Band1R;
    juce::IIRFilter preEQ_Band2L;   // 100Hz ピーク -1dB
    juce::IIRFilter preEQ_Band2R;
    juce::IIRFilter preEQ_Band3L;   // 200Hz ピーク -3dB
    juce::IIRFilter preEQ_Band3R;
    juce::IIRFilter preEQ_Band4L;   // 400Hz ピーク -4dB
    juce::IIRFilter preEQ_Band4R;
    juce::IIRFilter preEQ_Band5L;   // 1kHz ピーク +2dB
    juce::IIRFilter preEQ_Band5R;
    juce::IIRFilter preEQ_Band6L;   // 4kHz ピーク +6dB
    juce::IIRFilter preEQ_Band6R;
    juce::IIRFilter preEQ_Band7L;   // 10kHz ローパス
    juce::IIRFilter preEQ_Band7R;
};
