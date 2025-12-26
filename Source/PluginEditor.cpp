/*
  ==============================================================================
    PluginEditor.cpp
    プラグインUIの実装
  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
// コンストラクタ
DomeLiveSimulatorAudioProcessorEditor::DomeLiveSimulatorAudioProcessorEditor(
    DomeLiveSimulatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // ウィンドウサイズ
    setSize(400, 500);

    // カスタムLook and Feelを設定
    setLookAndFeel(&domeLookAndFeel);

    // ワンノブの設定
    domeKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    domeKnob.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    domeKnob.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(domeKnob);

    // パラメータにアタッチ
    domeKnobAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(), "domeAmount", domeKnob);

    // ラベル「DOME」
    domeLabel.setText("DOME", juce::dontSendNotification);
    domeLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    domeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    domeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(domeLabel);

    // 値表示ラベル
    valueLabel.setText("50%", juce::dontSendNotification);
    valueLabel.setFont(juce::Font(20.0f));
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00d4ff));
    valueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(valueLabel);

    // ノブの値が変わったらラベルを更新
    domeKnob.onValueChange = [this]()
    {
        int percentage = static_cast<int>(domeKnob.getValue() * 100);
        valueLabel.setText(juce::String(percentage) + "%", juce::dontSendNotification);
    };

    // 初期値を表示
    domeKnob.onValueChange();

    // プリセットラベル
    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaaaaaa));
    presetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetLabel);

    // プリセット選択コンボボックス
    presetSelector.addItem("Arena", 1);
    presetSelector.addItem("Stadium", 2);
    presetSelector.addItem("Hall", 3);
    presetSelector.addItem("Club", 4);
    presetSelector.setSelectedId(1);
    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a4a));
    presetSelector.setColour(juce::ComboBox::textColourId, juce::Colour(0xff00d4ff));
    presetSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00d4ff).withAlpha(0.5f));
    addAndMakeVisible(presetSelector);

    // プリセットパラメータにアタッチ
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getAPVTS(), "preset", presetSelector);
}

// デストラクタ
DomeLiveSimulatorAudioProcessorEditor::~DomeLiveSimulatorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
// 描画
void DomeLiveSimulatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 背景グラデーション
    juce::ColourGradient backgroundGradient(
        juce::Colour(0xff0a0a1a), 0, 0,
        juce::Colour(0xff1a1a3a), 0, (float)getHeight(), false
    );
    g.setGradientFill(backgroundGradient);
    g.fillAll();

    // タイトル
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xffffffff));
    g.drawText("DOME LIVE SIMULATOR", getLocalBounds().removeFromTop(60),
               juce::Justification::centred, true);

    // サブタイトル
    g.setFont(juce::Font(12.0f));
    g.setColour(juce::Colour(0xff888888));
    g.drawText("Arena Reverb Emulator", 0, 40, getWidth(), 30,
               juce::Justification::centred, true);

    // 装飾ライン
    g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.3f));
    g.drawLine(50, 70, getWidth() - 50, 70, 1.0f);

    // フッター
    g.setFont(juce::Font(10.0f));
    g.setColour(juce::Colour(0xff666666));
    g.drawText("v1.0.0", getLocalBounds().removeFromBottom(30),
               juce::Justification::centred, true);
}

// コンポーネントのレイアウト
void DomeLiveSimulatorAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // 中央にノブを配置
    auto knobSize = 220;
    auto knobArea = bounds.withSizeKeepingCentre(knobSize, knobSize);
    knobArea.setY(90);
    domeKnob.setBounds(knobArea);

    // ノブの下にラベル
    domeLabel.setBounds(bounds.withY(knobArea.getBottom()).withHeight(35));
    valueLabel.setBounds(bounds.withY(knobArea.getBottom() + 30).withHeight(25));

    // プリセットセレクター（下部に配置）
    auto presetY = knobArea.getBottom() + 75;
    presetLabel.setBounds((getWidth() - 200) / 2, presetY, 200, 20);
    presetSelector.setBounds((getWidth() - 150) / 2, presetY + 22, 150, 30);
}
