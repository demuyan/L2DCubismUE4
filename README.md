# Live2D Cubism Plugin for UnrealEngine 4

[Live2D Cubism](https://www.live2d.com/download/)で作成したモデルやモーション（動き）をUnrealEngine4で扱うためのプラグインです。


## 概要

本プラグインを利用することで[Live2D Cubism](https://www.live2d.com/download/)にて作成したモデルやモーションをUnrealEngine4上で表示できるようになります。

できることは次の通りです。

 - モデルデーター(.moc3ファイル、テクスチャー)やモーションデータがインポートできます
 - 独自のレンダラーによりモデルが表示できます
 - レベル上、HUDのいずれにも表示が可能です`1
 - Blueprintによるモーションの切り替えができます

## 使い方

プラグインを利用した[サンプルプロジェクト](http://github.com/demuyan/L2DCubismUE4Example)を参考にしてください。

## インストール

[インストールドキュメント](Docs/install.ja.md)を参考にしてください。

## 注意

本プラグインはLive2D社公式のプラグインでは*ありません*。

本プラグインを利用してLive2D Cubismのモデルやモーションを表示するアプリケーションをリリースする際は、Live2D社と契約を結ぶ必要があります。詳細は[Live2D社へお問い合わせください](https://www.live2d.jp/contact/)。

## 問い合わせ

本プラグインについての質問・問い合わせは、[こちら](https://discord.com/channels/734061312849477683/734061312849477686)までお願いします。

UE4の使い方等に関するお問い合わせは応じかねます。

## 動作環境

- Windows10 HOME(64bit)
- UE4.24.3
- VisualStudio 2017 CommunityEdition

## License

MIT license

## Auther

[Nari Demura](demuyan@gmail.com)

## 履歴

### 2020/07/31 v0.5.0
公開