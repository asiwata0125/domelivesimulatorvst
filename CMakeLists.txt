cmake_minimum_required(VERSION 3.22)

# プロジェクト名
project(DomeLiveSimulator VERSION 1.0.0)

# C++17を使用
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCEをサブディレクトリとして追加
add_subdirectory(C:/JUCE ${CMAKE_BINARY_DIR}/JUCE)

# オーディオプラグインの定義
juce_add_plugin(DomeLiveSimulator
    # プラグイン情報
    COMPANY_NAME "DomeLive"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Dome
    PLUGIN_CODE Dlsm
    FORMATS VST3 Standalone
    PRODUCT_NAME "Dome Live Simulator"
)

# ソースファイルの追加
target_sources(DomeLiveSimulator
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/DomeReverb.cpp
        Source/DSP/CombFilter.cpp
        Source/DSP/AllPassFilter.cpp
)

# JUCEのコンパイル定義
target_compile_definitions(DomeLiveSimulator
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# 必要なJUCEモジュールのリンク
target_link_libraries(DomeLiveSimulator
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
