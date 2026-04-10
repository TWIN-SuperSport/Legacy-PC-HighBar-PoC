# Legacy-PC-HighBar-PoC

LPSE を使った高鉄棒ゲームの PoC です。

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
