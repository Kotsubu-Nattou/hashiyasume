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
    // 説明：
    // 各式は、RGBからHSVへの変換式の「逆算」である。
    // 直接的なRGBに対し、HSVは「人間の感覚」に合わせて表現したもの。
    // 「max=v」の式を、「max＝明度」などと説明しても、明度が
    // どう数値化されたかは、RGB目線では分からず、意味がない。
    // 単純に「広げた風呂敷を手順どうりに直す」だけである。

    // RGBの中で最も大きい数を逆算
    //      v = max
    float max = v;

    // RGBの中で最も小さい数を逆算
    //      s = (max - min) / max
    //      s * max = max - min
    //      s * max + min = max
    float min = max - s * max;


    // @ RGBを逆算
    // 説明（H < 60 の場合）：
    // ・RとBは、Hの角度により、それぞれ最大値と最小値が決まっている。
    // ・G（(max-min)をdiffとする。bはminに相当）
    //   h = (g - b) / diff * 60
    //   h / 60 = (g - b) / diff
    //   h / 60 * diff = g - b
    //   h / 60 * diff + b = g
    //   g = b + diff * h / 60
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

    // RGBの中で最も大きい数を取得
    float max = r;
    if (max < g) max = g;
    if (max < b) max = b;

    // RGBの中で最も小さい数を取得
    float min = r;
    if (min > g) min = g;
    if (min > b) min = b;


    // @ 明度
    // 式の説明：
    // RGBの中の最も大きい数が、そのまま明度を表す
    v = max;


    // グレースケールなら色相と彩度を0として抜ける（maxのゼロチェックも兼ねる）
    if (min == max) {
        h = s = 0.0f;
        return;
    }


    // @ 彩度
    // 式の説明：
    // まずRGB各値の上下差を求め、次にその「高さ」で割る。
    // 例、RGB差が0.2のとき、高さ0.2なら結果は1.0となり「高彩度」と言える。
    // 高さ0.5なら結果は0.4となり「中彩度」と言える。高さ1.0なら結果は0.2
    // となり「低彩度」と言える。
    s = (max - min) / max;


    // @ 色相
    // RGBのどれが最大値かによって、場合分け処理。
    // 式の説明：
    // 色相環における位置を求める（0～360の度数。R=0度とする）
    // ・Rが最大値の場合
    //   まずRを、基準の軸（0度）として考える。
    //   (g - b)で、GとBの強さの「GBベクトル」を求める（Gが強いなら0～1の正、Bが強いなら0～-1の負）
    //   「GBベクトル」を「RGB全体の差」で割る。これにより「Rを基点としたベクトル」-1～1が求まる。
    //   最後に、60.0fを掛けて「度数に変換」。
    //   例： Gが1、Bが0なら結果は60度（黄色）となる。Gが0、Bが1なら結果は-60で300度（紫）となる。
    // ・GやBが最大値の場合
    //   上記に加え、それぞれ基準を0度として計算するため、360度圏における「在るべき位置」に移動させる。
    if      (r == max) {
        h = (g - b) / (max-min) * 60.0f;
        if (h < 0.0f) h += 360.0f;        // もし結果が負数なら、正数に直す
    }
    else if (g == max)
        h = (b - r) / (max-min) * 60.0f + 120.0f;
    else // (b == max)
        h = (r - g) / (max-min) * 60.0f + 240.0f;


    return;
}
