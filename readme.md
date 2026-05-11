# Cheap Tester (Raspberry Pi Pico / Pico2)

Raspberry Pi Pico / Pico2 を使った **簡易テスター** です。  
抵抗値と電圧を測定し、I²C接続のキャラクタLCDに表示します。

---

## 機能

### 1) 抵抗測定モード（RES）
- 自動レンジ切替（1kΩ / 10kΩ / 100kΩ / 1MΩ）
- LCDに kΩ 表示（小数点付き）
- 低抵抗時（目安 80Ω以下）にブザーON

### 2) 電圧測定モード（VOLT）
- 固定分圧（約 1:11）で測定
- LCD表示: `±xx.xxV`
- 回路としては正電圧測定を想定

### 3) モード時の主要制御ピン状態
| ピン | 抵抗測定時 | 電圧測定時 |
|---|---|---|
| Q8 (GP4) | ON | OFF |
| SHDN_GP12 (GP12, Active-Low) | 1 (有効) | 0 (シャットダウン) |

---

## GPIO定義（`Inc/config.h`）

```c
#define R1_GPIO      13
#define R2_GPIO      10
#define R3_GPIO       8
#define R4_GPIO       6
#define MOS_GP3       3
#define MOS_GP2       2
#define MOS_GP14     14
#define SHDN_GP12    12
#define Q8_GPIO       4
#define BUZZER_GPIO   5
#define LED_GPIO     25
#define SW_RES_GPIO  17
#define SW_VOL_GPIO  18

#define I2C_PORT  i2c1
#define PIN_SDA   26
#define PIN_SCL   27
#define LCD_ADDR  0x3E
#define ADC_ADDR  0x68
```

| 機能 | シンボル | GPIO |
|---|---|---|
| 抵抗レンジ1 | `R1_GPIO` | 13 |
| 抵抗レンジ2 | `R2_GPIO` | 10 |
| 抵抗レンジ3 | `R3_GPIO` | 8 |
| 抵抗レンジ4 | `R4_GPIO` | 6 |
| モード制御MOS | `MOS_GP3` | 3 |
| モード制御MOS | `MOS_GP2` | 2 |
| モード制御MOS | `MOS_GP14` | 14 |
| SHDN制御 | `SHDN_GP12` | 12 |
| Q8制御 | `Q8_GPIO` | 4 |
| ブザー | `BUZZER_GPIO` | 5 |
| LED | `LED_GPIO` | 25 |
| RESスイッチ | `SW_RES_GPIO` | 17 |
| VOLスイッチ | `SW_VOL_GPIO` | 18 |
| I²C SDA | `PIN_SDA` | 26 |
| I²C SCL | `PIN_SCL` | 27 |

---

## ディレクトリ構成

```text
Cheap_Tester/
├── readme.md
├── Inc/
│   ├── config.h
│   ├── hw_adc.h
│   ├── hw_gpio.h
│   ├── hw_i2c.h
│   ├── hw_lcd.h
│   └── measure.h
├── Src/                     # ビルド対象
│   ├── main.c
│   ├── measure.c
│   ├── hw_adc.c
│   ├── hw_gpio.c
│   ├── hw_i2c.c
│   └── hw_lcd.c
└── anser/                   # 学習用の解答例（通常ビルド対象外）
    ├── main_ans.c
    └── measure_ans.c
```

---

## ビルド

Pico SDK が使える環境で:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

`PICO_SDK_PATH` が必要な環境では、事前に設定してください。

---

## メモ
- `Src/` が実装本体です。
- `anser/` は学習用の参考コードです。
