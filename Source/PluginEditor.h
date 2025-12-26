/*
  ==============================================================================
    PluginEditor.h
    プラグインのUI（グラフィカルインターフェース）を担当
    
    ダークテーマのUIに大きなワンノブを配置。
    ドームのステージ照明をイメージしたデザイン。
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// カスタムLook and Feel（ノブのデザイン）
class DomeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DomeLookAndFeel()
    {
        // ダークテーマの色設定
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00d4ff));     // シアン
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff1a1a2e)); // ダークブルー
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffff00ff));               // マゼンタ
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 10.0f;
        auto centreX = (float)x + (float)width * 0.5f;
        auto centreY = (float)y + (float)height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // 外側のリング（グラデーション）
        juce::ColourGradient gradient(
            juce::Colour(0xff0a0a1a), centreX, centreY - radius,
            juce::Colour(0xff1a1a3a), centreX, centreY + radius, false
        );
        g.setGradientFill(gradient);
        g.fillEllipse(rx, ry, rw, rw);

        // 内側の円
        auto innerRadius = radius * 0.75f;
        juce::ColourGradient innerGradient(
            juce::Colour(0xff2a2a4a), centreX, centreY - innerRadius,
            juce::Colour(0xff1a1a2e), centreX, centreY + innerRadius, false
        );
        g.setGradientFill(innerGradient);
        g.fillEllipse(centreX - innerRadius, centreY - innerRadius, 
                      innerRadius * 2, innerRadius * 2);

        // アーク（値の表示）
        juce::Path arc;
        arc.addCentredArc(centreX, centreY, radius - 5, radius - 5,
                          0.0f, rotaryStartAngle, angle, true);
        
        // グロー効果のあるアーク
        juce::ColourGradient arcGradient(
            juce::Colour(0xff00d4ff), centreX, centreY - radius,
            juce::Colour(0xffff00ff), centreX, centreY + radius, false
        );
        g.setGradientFill(arcGradient);
        g.strokePath(arc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        // ノブのポインター
        juce::Path pointer;
        auto pointerLength = radius * 0.6f;
        auto pointerThickness = 4.0f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -pointerLength,
                                     pointerThickness, pointerLength * 0.6f, 2.0f);
        g.setColour(juce::Colour(0xffffffff));
        g.fillPath(pointer, juce::AffineTransform::rotation(angle)
                                                   .translated(centreX, centreY));

        // 中央のドット
        g.setColour(juce::Colour(0xff00d4ff));
        g.fillEllipse(centreX - 8, centreY - 8, 16, 16);
    }
};

//==============================================================================
// プラグインエディター（メインUI）
class DomeLiveSimulatorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DomeLiveSimulatorAudioProcessorEditor(DomeLiveSimulatorAudioProcessor&);
    ~DomeLiveSimulatorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    DomeLiveSimulatorAudioProcessor& audioProcessor;

    // カスタムLook and Feel
    DomeLookAndFeel domeLookAndFeel;

    // ワンノブ
    juce::Slider domeKnob;
    juce::Label domeLabel;
    juce::Label valueLabel;

    // プリセット選択
    juce::ComboBox presetSelector;
    juce::Label presetLabel;

    // パラメータアタッチメント（UIとパラメータを同期）
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> domeKnobAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DomeLiveSimulatorAudioProcessorEditor)
};
