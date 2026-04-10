# Legacy-PC-HighBar-PoC WebAssembly化メモ

## 概要
- `Legacy-PC-HighBar-PoC` を `SDL2 + Emscripten` で Web 化し、Ubuntu ローカルブラウザ上で動作確認した。
- 出力先は `/home/twin_supersport/www/temp/wasm-test/highbar`。

## 実装上の要点
- ネイティブ版とは別に、Web 版専用の `main_web.c` を用意した。
- イベントループは `emscripten_set_main_loop_arg` ベースへ置き換えた。
- フォントは `DejaVuSansMono.ttf` を `--preload-file` で読み込む形にした。

## 入力の安定化
- Web 版では `SDL_KEYDOWN` 経由の入力が安定しなかった。
- そのため `SPACE` `R` `Q` は、`index.html` 側の `keydown` から `Module.ccall('web_key_event', ...)` で C 側へ直接渡す形に変更した。
- `SDL_PollEvent()` 側では `SDL_QUIT` だけを残し、キー入力の二重処理を避けた。

## 表示の安定化
- iPhone の `RDP` 経由では、canvas の縮小表示で文字欠けが発生した。
- 対応として、Web 側で整数倍率ベースのスケーリングへ変更した。
- 最終的には Web 版のフォントサイズを `13` に落とすことで、iPhone `RDP` 上でも収まりが改善した。

## 表示倍率 UI
- 上部に `Display scale` UI を追加した。
- 現在は
  - `AUTO`
  - `UP`
  - `DOWN`
 で `1.0x .. 2.0x` を `0.1` 刻みで調整できる。

## 今の状態
- Ubuntu ローカルブラウザで動作確認済み。
- iPhone `RDP` 経由でも表示倍率を調整すればプレイ可能。
- 試験公開先 `https://supersport-life.upper.jp/` でも動作確認済み。
- 配信物は `index.html`, `highbar.js`, `highbar.wasm`, `highbar.data` の 4 ファイルを同一ディレクトリへ置く形で成立した。
- 現時点の Web 版実装は、一時配置からの運用であり、本 repo への本格統合は今後の検討事項。
