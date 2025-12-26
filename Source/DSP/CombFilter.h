/*
  ==============================================================================
    CombFilter.h
    コムフィルター - リバーブの基本ブロック
    
    コムフィルターは入力信号を遅延させてフィードバックするフィルター。
    遅延時間とフィードバック量によってリバーブの特性が決まる。
  ==============================================================================
*/

#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class CombFilter
{
public:
    CombFilter() = default;
    ~CombFilter() = default;

    // サンプルレートと最大遅延時間でバッファを初期化
    void prepare(double newSampleRate, float maxDelayMs = 200.0f)
    {
        sampleRate = newSampleRate;
        int maxDelaySamples = static_cast<int>(maxDelayMs * sampleRate / 1000.0);
        buffer.resize(maxDelaySamples, 0.0f);
        writeIndex = 0;
    }

    // 遅延時間を設定（ミリ秒）
    void setDelayTime(float delayMs)
    {
        delaySamples = static_cast<int>(delayMs * sampleRate / 1000.0);
        if (delaySamples >= buffer.size())
            delaySamples = static_cast<int>(buffer.size()) - 1;
        if (delaySamples < 1)
            delaySamples = 1;
    }

    // フィードバック量を設定（0.0 - 0.99）
    void setFeedback(float fb)
    {
        feedback = std::clamp(fb, 0.0f, 0.99f);
    }

    // ダンピング（高域減衰）を設定
    void setDamping(float damp)
    {
        damping = std::clamp(damp, 0.0f, 1.0f);
    }

    // 1サンプル処理
    float process(float input)
    {
        // 読み取り位置を計算
        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0)
            readIndex += static_cast<int>(buffer.size());

        // 遅延信号を取得
        float delayed = buffer[readIndex];

        // ローパスフィルター（ダンピング）を適用
        filterStore = delayed * (1.0f - damping) + filterStore * damping;

        // フィードバック付きで書き込み
        buffer[writeIndex] = input + filterStore * feedback;

        // 書き込み位置を進める
        writeIndex++;
        if (writeIndex >= buffer.size())
            writeIndex = 0;

        return delayed;
    }

    // バッファをクリア
    void clear()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        filterStore = 0.0f;
    }

private:
    std::vector<float> buffer;
    double sampleRate = 44100.0;
    int writeIndex = 0;
    int delaySamples = 1;
    float feedback = 0.7f;
    float damping = 0.5f;
    float filterStore = 0.0f;
};
