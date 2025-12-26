/*
  ==============================================================================
    AllPassFilter.h
    オールパスフィルター - 音を拡散させる
    
    オールパスフィルターは全ての周波数を同じゲインで通過させるが、
    位相を変化させる。これによりリバーブの密度を高める。
  ==============================================================================
*/

#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class AllPassFilter
{
public:
    AllPassFilter() = default;
    ~AllPassFilter() = default;

    // サンプルレートと最大遅延時間でバッファを初期化
    void prepare(double newSampleRate, float maxDelayMs = 100.0f)
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

    // フィードバック係数を設定（通常0.5程度）
    void setCoefficient(float coeff)
    {
        coefficient = std::clamp(coeff, 0.0f, 0.9f);
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

        // オールパスフィルターの計算
        // y[n] = -g * x[n] + x[n-d] + g * y[n-d]
        float output = -coefficient * input + delayed;
        buffer[writeIndex] = input + coefficient * delayed;

        // 書き込み位置を進める
        writeIndex++;
        if (writeIndex >= buffer.size())
            writeIndex = 0;

        return output;
    }

    // バッファをクリア
    void clear()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
    }

private:
    std::vector<float> buffer;
    double sampleRate = 44100.0;
    int writeIndex = 0;
    int delaySamples = 1;
    float coefficient = 0.5f;
};
