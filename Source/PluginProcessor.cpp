/*
  ==============================================================================
    PluginProcessor.cpp
    プラグインのオーディオ処理を担当するメインクラスの実装
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// コンストラクタ
DomeLiveSimulatorAudioProcessor::DomeLiveSimulatorAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

// デストラクタ
DomeLiveSimulatorAudioProcessor::~DomeLiveSimulatorAudioProcessor()
{
}

//==============================================================================
// パラメータレイアウトを作成
juce::AudioProcessorValueTreeState::ParameterLayout 
DomeLiveSimulatorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ワンノブパラメータ「Dome Amount」
    // 0% = ドライ、100% = 最大リバーブ
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("domeAmount", 1),  // パラメータID
        "Dome Amount",                        // 表示名
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),  // 範囲
        0.5f                                  // デフォルト値
    ));

    // プリセット選択パラメータ
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("preset", 1),
        "Preset",
        juce::StringArray{ "Arena", "Stadium", "Hall", "Club" },
        0  // デフォルト: Arena
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
// オーディオ処理の準備
void DomeLiveSimulatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // リバーブを初期化
    domeReverb.prepare(sampleRate, samplesPerBlock);
    
    // 初期パラメータを設定
    float domeAmount = apvts.getRawParameterValue("domeAmount")->load();
    domeReverb.setDomeAmount(domeAmount);
}

// リソース解放
void DomeLiveSimulatorAudioProcessor::releaseResources()
{
    domeReverb.clear();
}

//==============================================================================
// オーディオブロック処理（毎フレーム呼ばれる）
void DomeLiveSimulatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                    juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    // 出力をクリア（ノイズ防止）
    juce::ScopedNoDenormals noDenormals;
    
    // 入力チャンネル数より出力チャンネル数が多い場合、余分をクリア
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // プリセットを取得してリバーブに設定
    int presetIndex = static_cast<int>(apvts.getRawParameterValue("preset")->load());
    if (presetIndex != currentPresetIndex)
    {
        currentPresetIndex = presetIndex;
        domeReverb.setPreset(static_cast<DomePreset>(presetIndex));
    }

    // パラメータを取得してリバーブに設定
    float domeAmount = apvts.getRawParameterValue("domeAmount")->load();
    domeReverb.setDomeAmount(domeAmount);

    // リバーブ処理
    domeReverb.process(buffer);
}

//==============================================================================
// ステート情報の保存（DAWがプロジェクトを保存するとき）
void DomeLiveSimulatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

// ステート情報の復元（DAWがプロジェクトを開くとき）
void DomeLiveSimulatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
// エディター（UI）を作成
juce::AudioProcessorEditor* DomeLiveSimulatorAudioProcessor::createEditor()
{
    return new DomeLiveSimulatorAudioProcessorEditor(*this);
}

//==============================================================================
// プリセット関連
void DomeLiveSimulatorAudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < 4)
    {
        currentPresetIndex = index;
        domeReverb.setPreset(static_cast<DomePreset>(index));
        
        // パラメータも更新
        if (auto* param = apvts.getParameter("preset"))
            param->setValueNotifyingHost(static_cast<float>(index) / 3.0f);
    }
}

const juce::String DomeLiveSimulatorAudioProcessor::getProgramName(int index)
{
    switch (index)
    {
        case 0: return "Arena";
        case 1: return "Stadium";
        case 2: return "Hall";
        case 3: return "Club";
        default: return {};
    }
}

//==============================================================================
// プラグインインスタンスを作成する関数（JUCEが呼び出す）
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DomeLiveSimulatorAudioProcessor();
}
