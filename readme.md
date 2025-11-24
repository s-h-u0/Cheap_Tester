# Cheap Tester (Raspberry Pi Pico / Pico2)

Raspberry Pi Pico / Pico2 を使った **簡易 LCR テスター風のツール** です。  
抵抗値と電圧を測定して、I²C 接続のキャラクタ LCD に表示します。

このリポジトリの目的は **「高校生・初心者が C とマイコンを学ぶ教材」** であることです。

- 実際に動くハードウェア
- 読みやすい C コード
- 穴埋め形式の **クイズ版ソースコード**
- きちんと動く **模範解答ソースコード**

を同じプロジェクトの中にまとめています。

---

## 機能概要

- 抵抗測定モード（RES モード）
  - 自動レンジ  
    1 kΩ / 10 kΩ / 100 kΩ / 1 MΩ のリファレンス抵抗を切り替えて測定
  - kΩ 表記・小数点付きで LCD 表示
  - 低抵抗（例：80 Ω 以下）のときにブザー ON（導通チェッカ風）

- 電圧測定モード（VOLT モード）
  - 固定分圧（約 1:11）で電圧を測定
  - 表示形式は `±xx.xx V`
  - 正負両方向の電圧測定（理論上 ±約 22 V 程度まで）

- その他
  - モード切り替え用のトグルスイッチ　1個
  - ステータス確認用 LED（Pico のオンボード LED）

---

## ハードウェア構成

### 必要な部品

- Raspberry Pi Pico / Pico2 本体
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
- プッシュスイッチ 2 個（RES / VOL モード）
- 必要な配線・基板など

### GPIO 割り当て

`Inc/config.h` で GPIO 割り当てを定義しています。

```c
/* --- GPIO ------------------------------------------------------------- */
#define R1_GPIO      13
#define R2_GPIO      10
#define R3_GPIO       8
#define R4_GPIO       6
#define MOS_GP3       3
#define MOS_GP2       2
#define MOS_GP14     14
#define MOS_GP12     12
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



## ドキュメント

- [回路図 (PDF)](README/schematic_CheapTester.pdf)
- [部品表 BOM (PDF)](README/CheapTester_BOM.pdf)
- [動作フローチャート (PDF)](README/CheapTester_Flow.pdf)
