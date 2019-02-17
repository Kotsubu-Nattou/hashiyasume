///////////////////////////////////////////////////////////////////////////////////////////////
// 【クラス定義】conv_hsv
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



class CLASS_HSV_2_RGB
{
    private:
        float h, s, v;

    public:
        void setH(const float hue);
        void setS(const float Saturation);
        void setV(const float Value);
        void getRGB(float &r, float &g, float &b);
};





class CLASS_RGB_2_HSV
{
    private:
        float r, g, b;

    public:
        void setR(const float red);
        void setG(const float green);
        void setB(const float blue);
        void getHSV(float &h, float &s, float &v);
};
