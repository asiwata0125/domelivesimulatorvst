/*
  ==============================================================================
    PluginProcessor.h
    プラグインのオーディオ処理を担当するメインクラス
    
    このクラスはDAWとのインターフェースを担当し、
    DomeReverbクラスを使って実際の音声処理を行う。
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DSP/DomeReverb.h"

class DomeLiveSimulatorAudioProcessor : public juce::AudioProcessor
{
public:
    //==========================================================================
    // コンストラクタ・デストラクタ
    DomeLiveSimulatorAudioProcessor();
    ~DomeLiveSimulatorAudioProcessor() override;

    //==========================================================================
    // オーディオ処理
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==========================================================================
    // プラグイン情報
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; } // リバーブのテール
    
    //==========================================================================
    // プログラム（プリセット）
    int getNumPrograms() override { return 4; }
    int getCurrentProgram() override { return currentPresetIndex; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    //==========================================================================
    // ステート保存/復元
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==========================================================================
    // エディター
    bool hasEditor() const override { return true; }
    juce::AudioProcessorEditor* createEditor() override;

    //==========================================================================
    // パラメータ
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    // パラメータツリーを作成
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // オーディオパラメータ
    juce::AudioProcessorValueTreeState apvts;

    // ドームリバーブ
    DomeReverb domeReverb;
    
    // 現在のプリセットインデックス
    int currentPresetIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DomeLiveSimulatorAudioProcessor)
};
