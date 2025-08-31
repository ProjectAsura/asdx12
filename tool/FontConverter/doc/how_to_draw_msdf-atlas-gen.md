`msdf-atlas-gen`を使って生成したフォントデータをDirectX12のスプライト描画に組み込むには、**MSDF AtlasのJSONメタデータからグリフ情報を取得し、それを元にスプライトの位置・サイズ・UV座標を計算**する必要があります。

以下に、MSDF AtlasのJSONデータから**グリフの位置・サイズ・テクスチャ座標を取得して描画**する一連の流れを、C++とDirectX12に沿って説明します。

---

## 🔧 前提

* `msdf-atlas-gen`で **JSON形式のメタデータ付き** atlas を生成済み（形式：`json` or `json-full`）
* DirectX12で**2Dスプライト描画の基盤が完成している**

---

## 📁 JSONメタデータの例（`json`形式）

```json
{
  "atlas": {
    "type": "msdf",
    "distanceRange": 4,
    "size": [512, 512],
    "width": 512,
    "height": 512,
    "yOrigin": "bottom"
  },
  "glyphs": [
    {
      "unicode": 65,
      "advance": 32,
      "planeBounds": { "left": -1, "bottom": -1, "right": 1, "top": 1 },
      "atlasBounds": { "left": 10, "bottom": 10, "right": 42, "top": 42 }
    },
    ...
  ]
}
```

---

## 🧩 グリフの描画情報の計算

### 必要な構造体（例）

```cpp
struct Glyph {
    float advance;
    float planeLeft, planeTop, planeRight, planeBottom;
    float atlasLeft, atlasTop, atlasRight, atlasBottom;
};
```

### 1. JSONの読み込み

RapidJSONなどを使ってJSONファイルを読み込み、必要なデータを構造体に格納します。

### 2. テクスチャ座標の計算（UV）

JSONの`atlasBounds`は、ピクセル単位で表現されています。これをUV（0～1）に変換します：

```cpp
float u0 = glyph.atlasLeft   / atlasWidth;
float v0 = glyph.atlasTop    / atlasHeight;
float u1 = glyph.atlasRight  / atlasWidth;
float v1 = glyph.atlasBottom / atlasHeight;
```

> 🔁 注意：「yOrigin」が`bottom`なら、Y座標を反転してください（DirectXは通常`top-left`が原点）

```cpp
// yOriginが"bottom"の場合、Y座標を反転する
v0 = 1.0f - (glyph.atlasTop / atlasHeight);
v1 = 1.0f - (glyph.atlasBottom / atlasHeight);
```

---

### 3. スプライトのスクリーン座標計算

JSONの`planeBounds`はフォントのem単位（通常1em = フォントの高さ）です。

```cpp
float scale = fontPixelSize; // 例えば、32pxなど

float spriteLeft   = glyph.planeLeft   * scale;
float spriteTop    = glyph.planeTop    * scale;
float spriteRight  = glyph.planeRight  * scale;
float spriteBottom = glyph.planeBottom * scale;

float width  = spriteRight - spriteLeft;
float height = spriteTop - spriteBottom;
```

* この情報を使って、スクリーン上に`(x + spriteLeft, y + spriteTop)`の位置に`width x height`のスプライトを描画します。

---

### 4. スプライト描画処理（擬似コード）

```cpp
DrawSprite(
    texture, 
    x + spriteLeft, y - spriteTop, // 注意：上下方向の反転
    width, height,
    u0, v0, u1, v1
);
```

---

## 📝 補足情報

| 項目            | 内容                            |
| ------------- | ----------------------------- |
| `advance`     | 次の文字のx座標に加えるオフセット（カーニング等を除いた） |
| `planeBounds` | フォント空間（em単位）におけるグリフの位置とサイズ    |
| `atlasBounds` | テクスチャ上のMSDF画像の矩形（ピクセル単位）      |

---

## ✅ まとめ

MSDF Atlasの`json`メタデータからスプライト描画に必要なデータを得るには：

1. JSONをパースして各グリフの`planeBounds`と`atlasBounds`を取得
2. `atlasBounds`を使ってUV座標を計算
3. `planeBounds`を使ってスプライトの表示サイズとオフセットを計算
4. DirectX12でスプライトを描画（必要に応じてMSDF用のシェーダーを使用）

---

## 🖼️ MSDF用のシェーダー

MSDF（距離フィールド）は専用のピクセルシェーダーが必要です。必要であれば、MSDF用のHLSLシェーダーも提供できます。

---

ご希望があれば、以下も対応可能です：

* HLSLのMSDFシェーダーコード
* JSONを読むためのコード（C++/RapidJSON）
* フル描画パイプラインのサンプル

必要な部分を教えてください。
