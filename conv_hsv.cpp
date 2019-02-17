///////////////////////////////////////////////////////////////////////////////////////////////
// 【クラス実装】conv_hsv
//
// ・内容と使い方
//   CLASS_HSV_2_RGB  ---  HSVをセットし、RGBをゲット
//   CLASS_RGB_2_HSV  ---  RGBをセットし、HSVをゲット
//
// ・各数値の範囲（すべてfloat）
//   RGB              : 0.0 ～ 1.0
//   Hue（色相）       : 0.0 ～ 360.0
//   Saturation（彩度）: 0.0 ～ 1.0
//   Value（明度）     : 0.0 ～ 1.0
///////////////////////////////////////////////////////////////////////////////////////////////



#include "conv_hsv.hpp"





///////////////////////////////////////////////////////////////////////////////////////////////
// CLASS_HSV_2_RGB
///////////////////////////////////////////////////////////////////////////////////////////////





void
CLASS_HSV_2_RGB::setH(const float hue) {
    // 【セッタ】色相
    h = hue;
    return;
}

void
CLASS_HSV_2_RGB::setS(const float Saturation) {
    // 【セッタ】彩度
    s = Saturation;
    return;
}

void
CLASS_HSV_2_RGB::setV(const float Value) {
    // 【セッタ】明度
    v = Value;
    return;
}





void
CLASS_HSV_2_RGB::getRGB(float &r, float &g, float &b)
{
    // 【ゲッタ】HSVをRGBに変換して返す
    // ＜戻り値＞ RGB: 0.0 ～ 1.0

    // @ 最大値と最小値を求める
    float max = v;
    float min = max - s * max;


    // @ RGBを求める
    if (h < 60.0f) {
        r = max;
        g = min + (max-min) * h / 60.0f;
        b = min;
    }
    else if (h < 120.0f) {
        r = min + (max-min) * (120.0f-h) / 60.0f;
        g = max;
        b = min;
    }
    else if (h < 180.0f) {
        r = min;
        g = max;
        b = min + (max-min) * (h-120.0f) / 60.0f;
    }
    else if (h < 240.0f) {
        r = min;
        g = min + (max-min) * (240.0f-h) / 60.0f;
        b = max;
    }
    else if (h < 300.0f) {
        r = min + (max-min) * (h-240.0f) / 60.0f;
        g = min;
        b = max;
    }
    else {  // (h < 360.0f)
        r = max;
        g = min;
        b = min + (max-min) * (360.0f-h) / 60.0f;
    }


    return;
}





///////////////////////////////////////////////////////////////////////////////////////////////
// CLASS_RGB_2_HSV
///////////////////////////////////////////////////////////////////////////////////////////////





void
CLASS_RGB_2_HSV::setR(const float red) {
    // 【セッタ】赤
    r = red;
    return;
}

void
CLASS_RGB_2_HSV::setG(const float green) {
    // 【セッタ】緑
    g = green;
    return;
}

void
CLASS_RGB_2_HSV::setB(const float blue) {
    // 【セッタ】青
    b = blue;
    return;
}





void
CLASS_RGB_2_HSV::getHSV(float &h, float &s, float &v)
{
    // 【ゲッタ】RGBをHSVに変換して返す
    // ＜戻り値＞ H: 0.0 ～ 360.0,  SV: 0.0 ～ 1.0

    // rgbの中で最も大きい数を取得
    float max = r;
    if (max < g) max = g;
    if (max < b) max = b;

    // rgbの中で最も小さい数を取得
    float min = r;
    if (min > g) min = g;
    if (min > b) min = b;


    // @ 明度
    v = max;


    // グレースケールなら色相と彩度は0（maxのゼロチェックも兼ねる）
    if (min == max) {
        h = s = 0.0f;
        return;
    }


    // @ 彩度
    s = (max - min) / max;


    // @ 色相
    // rgbのどれが最大値かによって、場合分け
    if      (r == max)
        h = (g - b) / (max-min) * 60.0f;
    else if (g == max)
        h = (b - r) / (max-min) * 60.0f + 120.0f;
    else // (b == max)
        h = (r - g) / (max-min) * 60.0f + 240.0f;
    
    // もし結果がマイナス値なら範囲内に補正
    if (h < 0.0f) h += 360.0f;


    return;
}
