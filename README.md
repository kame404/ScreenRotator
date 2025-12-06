# Screen Rotator

<img width="1009" height="450" alt="screenshot" src="https://github.com/user-attachments/assets/23541d80-c8b2-47a3-9869-8d6e3b5c5d0c" />

「Ctrl + Alt + 矢印キー」による画面回転機能を、Windows OS標準APIのみを用いて再現するユーティリティです。

C言語とWin32 APIで記述されており、ランタイムやフレームワークに依存しません。

## 特徴

*   実行ファイルサイズは約19KB、メモリ使用量は極小（約0.1MB）です。
*   マウスカーソルが存在するモニターを自動判定して回転させます。
*   引数なしで常駐アプリ、引数ありでCLIツールとして動作します。
*   Per-Monitor DPI Aware V2 に対応し、スケーリングの異なるマルチモニター環境でも座標ズレを防ぎます。

## ダウンロード

[Releases](https://github.com/kame404/ScreenRotator/releases/)よりコンパイル済みのバイナリ（exe）をダウンロード可能ですが、本ツールはソースコードを確認し、ご自身でビルドして使用することを推奨します。

## ビルド方法

MinGW (GCC) 環境にて、以下のコマンドでビルド可能です。

```bash
gcc -o ScreenRotator.exe ScreenRotator.c -mwindows -static -Os -s
```

*   `-mwindows`: GUIアプリケーションとしてビルド（コンソール非表示）
*   `-static`: 必要なライブラリを静的リンク
*   `-Os`: サイズ最適化
*   `-s`: デバッグ情報の削除

## 使い方

### 常駐モード（GUI）

`ScreenRotator.exe` を引数なしで実行するとタスクトレイに常駐します。

**ホットキー:**
*   `Ctrl` + `Alt` + `Up (↑)` : 通常（0度）
*   `Ctrl` + `Alt` + `Down (↓)` : 反転（180度）
*   `Ctrl` + `Alt` + `Left (←)` : 左回転（270度）
*   `Ctrl` + `Alt` + `Right (→)` : 右回転（90度）

※ マウスカーソルが置かれているモニターが操作対象となります。

**終了方法:**
タスクトレイのアイコンを右クリックし、「Exit」を選択してください。

### コマンドラインモード（CLI）

引数を指定して実行することで、バッチファイル等から画面回転を制御できます。この場合、常駐はせず処理完了後に終了します。

**構文:**
```cmd
ScreenRotator.exe [option] [monitor]
```

**オプション:**
*   `/up`, `/down`, `/left`, `/right` : 回転方向を指定

**モニター指定:**
*   (指定なし) : プライマリモニター
*   `/display:N` : モニター番号 N を指定 (1, 2, ...)

**使用例:**
```cmd
REM モニター2を右に回転
ScreenRotator.exe /right /display:2
```

## 開発について

本ツールの開発（C言語およびWin32 APIの実装）には、Claude 4.5 Opus を活用しています。
