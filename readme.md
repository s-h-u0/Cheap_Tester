# Cheap Tester (Raspberry Pi Pico / Pico2)

Raspberry Pi Pico / Pico2 を使った **簡易 テスター** です。  
抵抗値と電圧を測定して、I²C 接続のキャラクタ LCD に表示します。

---

## 機能概要

- **抵抗測定モード（RES モード）**
  - 自動レンジ  
    1 kΩ / 10 kΩ / 100 kΩ / 1 MΩ のリファレンス抵抗を切り替えて測定
  - kΩ 表記・小数点付きで LCD 表示
  - 低抵抗（例：80 Ω 以下）のときにブザー ON（導通チェッカ風）

- **電圧測定モード（VOLT モード）**
  - 固定分圧（約 1:11）で電圧を測定
  - 表示形式は `±xx.xx V`
  - 正方向の電圧測定（理論上 + 約 22 V 程度まで）※mcp3425は正負測れるADCでソフトもそうなってますが、回路は正電源のみ測定用になってます。

- **その他**
  - モード切り替え用のロッカースイッチ(トグル)1個
  - 測定レンジ切替・MOSFET 駆動
  - ステータス確認用 LED（Pico / Pico2 のオンボード LED）

---

## ハードウェア構成

### 必要な部品（目安）

- Raspberry Pi Pico2 本体
- I²C キャラクタ LCD  
  - AQM0802A + I²C ブリッジ（アドレス: `0x3E`）を想定
- ADC（高分解能）
  - MCP3425（アドレス: `0x68`）
- リファレンス抵抗
  - 1 kΩ
  - 10 kΩ
  - 100 kΩ
  - 1 MΩ  
  （値は `measure.c` の計算式に合わせています）
- Nch MOSFET 数個（レンジ切替・測定モード切替用）
- 圧電ブザー
- トグルスイッチ 1 個（RES / VOL モード）
- 必要な配線・基板など

### GPIO 割り当て

`Inc/config.h` で GPIO 割り当てを定義しています（抜粋）。

```c
/* --- GPIO ------------------------------------------------------------- */
#define R1_GPIO      13
#define R2_GPIO      10
#define R3_GPIO       8
#define R4_GPIO       6
#define MOS_GP3       3
#define MOS_GP2       2
#define MOS_GP14     14
#define SHDN_GP12     12
#define BUZZER_GPIO   5
#define LED_GPIO     25
#define SW_RES_GPIO  17
#define SW_VOL_GPIO  18

/* --- I²C -------------------------------------------------------------- */
#define I2C_PORT  i2c1
#define PIN_SDA   26
#define PIN_SCL   27
#define LCD_ADDR  0x3E    /* AQM0802A */
#define ADC_ADDR  0x68    /* MCP3425  */
```

| 機能              | シンボル        | GPIO |
|-------------------|-----------------|------|
| リファレンス抵抗1 | `R1_GPIO`       | 13   |
| リファレンス抵抗2 | `R2_GPIO`       | 10   |
| リファレンス抵抗3 | `R3_GPIO`       | 8    |
| リファレンス抵抗4 | `R4_GPIO`       | 6    |
| MOSFET 制御       | `MOS_GP3`       | 3    |
| MOSFET 制御       | `MOS_GP2`       | 2    |
| MOSFET 制御       | `MOS_GP14`      | 14   |
| 低電圧源/SHDN 制御       | `SHDN_GP12`      | 12   |
| ブザー            | `BUZZER_GPIO`   | 5    |
| ステータス LED    | `LED_GPIO`      | 25   |
| 抵抗モード ロッカSW     | `SW_RES_GPIO`   | 17   |
| 電圧モード ロッカSW     | `SW_VOL_GPIO`   | 18   |
| I²C SDA           | `PIN_SDA`       | 26   |
| I²C SCL           | `PIN_SCL`       | 27   |
| LCD アドレス      | `LCD_ADDR`      | 0x3E |
| ADC アドレス      | `ADC_ADDR`      | 0x68 |

---

## ドキュメント

ルート直下の `README` フォルダに、ハードウェアの資料をまとめています。

- [回路図 (schematic_CheapTester.pdf)](README/schematic_CheapTester.pdf)
- [部品表 BOM (CheapTester_BOM.pdf)](README/CheapTester_BOM.pdf)
- [動作フローチャート (CheapTester_Flow.pdf)](README/CheapTester_Flow.pdf)


---

## フォルダ構成

```text
Cheap_Tester/
├── README.md                 # このファイル
├── README/                   # PDF や図などの資料
│   ├── CheapTester_BOM.pdf
│   ├── CheapTester_Flow.pdf
│   └── schematic_CheapTester.pdf
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── .gitignore
├── .vscode/                  # VS Code 用設定
├── Inc/                      # ヘッダファイル
│   ├── config.h
│   ├── hw_adc.h
│   ├── hw_gpio.h
│   ├── hw_i2c.h
│   ├── hw_lcd.h
│   └── measure.h
├── Src/                      # ★ 学習用：クイズ版（ビルド対象）
│   ├── main.c
│   ├── measure.c
│   ├── hw_adc.c
│   ├── hw_gpio.c
│   ├── hw_i2c.c
│   └── hw_lcd.c
└── anser/                    # ★ 模範解答（ビルドされない）
    ├── main_ans.c
    └── measure_ans.c
```

- `Src/`  
  高校生・初心者向けの **穴埋めコード** を置くフォルダです。  
  CMakeLists.txt ではこのフォルダの `.c` だけをビルド対象にしています。

- `anser/`  
  動く **解答** を置くフォルダです。  
  CMakeLists.txt からは参照していないので、通常のビルドではコンパイルされません。

---

## ビルド方法（ざっくり）

Pico SDK のセットアップが済んでいる前提です。

```bash
mkdir build
cd build
cmake ..
ninja      # または make
```

できあがった `.uf2` を BOOTSEL モードの Pico / Pico2 にコピーすると書き込みできます。

---

## 学習用としての進め方

1. まずはクイズ版 (`Src/main.c`, `Src/measure.c`) をそのままビルドして、  
   実機で抵抗・電圧が測れることを確認する。
2. コメントを読みながら、少しずつ自分で書き換えたり、穴埋め部分を埋めてみる。
3. 分からなくなったら `anser/main_ans.c`, `anser/measure_ans.c` を見て、  
   自分のコードと一行ずつ見比べる。
4. 気づいたことや理解したことをコメントとしてコードに書き足しておくと、  
   後から見返したときにとても楽になります。
