///////////////////////////////////////////////////////////////////////////////////////////////
// 【クラス定義】conv_hsv
//
// ・内容と使い方
//   CLASS_HSV_2_RGB  ---  HSVをセットし、RGBをゲット
//   CLASS_RGB_2_HSV  ---  RGBをセットし、HSVをゲット
//
// ・各数値の有効範囲
//   RGB              : 0 ～ 1    (float)
//   Hue（色相）       : 0 ～ 360  (float, int)
//   Saturation（彩度）: 0 ～ 1    (float)
//   Value（明度）     : 0 ～ 1    (float)
///////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _INCLUDE_CONV_HSV_
#define _INCLUDE_CONV_HSV_





class CLASS_HSV_2_RGB
{
    private:
        float h, s, v;

    public:
        void setH(float hue);
        void setH(  int hue);  // int版オーバーロード
        void setS(float saturation);
        void setV(float value);
        void setHSV(float hue, float saturation, float value);
        void setHSV(  int hue, float saturation, float value);  // hueのint版オーバーロード
        void getRGB(float &r, float &g, float &b);
};





class CLASS_RGB_2_HSV
{
    private:
        float r, g, b;

    public:
        void setR(float red);
        void setG(float green);
        void setB(float blue);
        void setRGB(float red, float green, float blue);
        void getHSV(float &h, float &s, float &v);
        void getHSV(  int &h, float &s, float &v);  // hueのint版オーバーロード
};



#endif