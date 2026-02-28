# 2048_M5Stack
## M5Core2ライブラリプロジェクト
M5Core2のSD-Updaterを用いたライブラリ集を構築するプロジェクトの一環として実装されています。

## どんなゲーム

「2048」はオープンソースのシンプルで楽しいゲームです！

以下のM5Stack用プログラムを参考に、M5Core2に移植しています。

https://github.com/gabrielecirulli/2048
https://github.com/yangshun/2048-python

オリジナルはMPU9250センサーを使用しており、M5Stackを地面に対して水平に保持することとしていました。
この判定条件を調整し、より遊びやすくしています。

## 操作方法

- **ジャイロ（傾き）**: M5Core2を傾けて移動方向を選択
- **画面フリック**: 画面をスワイプして直接移動（上下左右対応）
- **Aボタン / Cボタン**: 移動方向の切り替え
- **Bボタン**: 選択した方向に移動を実行

## 特徴

- タイル移動時のスライドアニメーション
- ジャイロ操作と画面フリック操作の両対応
- 移動可能なマージが残っている限りゲーム続行（不正なゲームオーバーの防止）

## M5Stack-SD-Updater対応です

https://github.com/tobozo/M5Stack-SD-Updater
https://github.com/lovyan03/M5Stack_LovyanLauncher

## 依存ライブラリ

- [M5Core2](https://github.com/m5stack/M5Core2)
- [M5Stack-SD-Updater](https://github.com/tobozo/M5Stack-SD-Updater)

# プログラマ

尾和 東/ぽこちゃ技術枠
