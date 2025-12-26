# Dome Live Simulator VST3

🏟️ 東京ドームのようなアリーナ音響をエミュレートする VST3 プラグイン

![Build Status](https://github.com/YOUR_USERNAME/domelivesimulatorvst/actions/workflows/build.yml/badge.svg)

## 特徴

- **ワンノブ操作** - ノブを回すだけでドーム感を調整
- **真のステレオリバーブ** - L/R 独立処理で広大なステレオイメージ
- **4 つのプリセット** - Arena / Stadium / Hall / Club
- **プリ EQ** - リバーブ前の EQ カーブで音の明瞭さを確保
- **低域強化** - ドームらしい迫力のある低音

## インストール

1. [Releases](https://github.com/YOUR_USERNAME/domelivesimulatorvst/releases)から最新版をダウンロード
2. ZIP を解凍
3. `DomeLiveSimulator.vst3` フォルダを以下にコピー:
   - Windows: `C:\Program Files\Common Files\VST3\`
   - macOS: `/Library/Audio/Plug-Ins/VST3/`
4. DAW でプラグインをスキャン

## ビルド方法

### 必要なもの

- Visual Studio 2022
- [JUCE Framework](https://juce.com/)

### 手順

1. JUCE をダウンロードして `C:\JUCE` に展開
2. Projucer で `DomeLiveSimulator.jucer` を開く
3. 「Save Project and Open in IDE」をクリック
4. Visual Studio で「Release」「x64」を選択してビルド

## 技術仕様

- **プラグイン形式**: VST3
- **DSP アルゴリズム**: FDN (Feedback Delay Network) ベースのリバーブ
- **フィルター構成**:
  - 16x コムフィルター (L/R 独立)
  - 8x オールパスフィルター (L/R 独立)
  - 7 バンド プリ EQ

## ライセンス

MIT License
