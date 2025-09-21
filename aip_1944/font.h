
#ifndef FONT_H
#define FONT_H
// 在aip1944.h中添加
typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t (*data)[7]; // 指向字体数据数组的指针
} FontDef;

extern const FontDef Font_4x5;
extern const FontDef Font_5x7;
extern const FontDef Font_IMG_1;


#endif