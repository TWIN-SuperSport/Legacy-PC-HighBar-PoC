# Legacy-PC-HighBar-PoC

LPSE を使った高鉄棒ゲームの PoC です。
注: LPSE は 80x25 の ANSI 風セル画面を描くための画面層です。

## Attention

この repo だけで LPSE の画面コードは含んでいますが、`SDL2` と `SDL2_ttf` はシステム側に事前導入が必要です。

## Dependencies

Ubuntu 系では少なくとも次を先に入れてください。

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build libsdl2-dev libsdl2-ttf-dev
```

## Controls

- `SPACE`: 1回目で回転開始、2回目でリリース
- `R`: リセット
- `Q` / `Esc`: 終了

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Run

```bash
./build/legacy-pc-highbar-poc
```

## Web

- `SDL2 + Emscripten` による Web 版の試験公開を確認済みです。
- 試験公開先: `https://supersport-life.upper.jp/`
- 詳細は `docs/WebAssembly化メモ.md` を参照してください。
