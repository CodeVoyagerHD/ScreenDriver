// display_font.h
#ifndef DISPLAY_FONT_H
#define DISPLAY_FONT_H

#include <stdint.h>

// 字体大小定义
typedef enum {
    FONT_SIZE_6x8,    // 6x8 小字体
    FONT_SIZE_8x16,   // 8x16 标准字体
    FONT_SIZE_12x24,  // 12x24 大字体
} font_size_t;

// 字符显示模式
typedef enum {
    MODE_NORMAL,      // 正常模式
    MODE_INVERT,      // 反色模式
    MODE_OVERWRITE,   // 覆盖模式(默认)
    MODE_XOR          // 异或模式
} char_display_mode_t;

// 字模结构
typedef struct {
    uint8_t width;     // 字符宽度(像素)
    uint8_t height;    // 字符高度(像素)
    uint8_t bytes_per_char; // 每个字符占用的字节数
    const uint8_t *data;    // 字模数据指针
} font_t;

// 获取指定字体和大小的字模
const font_t* get_font(font_size_t size);

// 外部字模声明(实际数据在单独的字体文件中)
extern const uint8_t font_6x8_data[];
extern const uint8_t font_8x16_data[];
extern const uint8_t font_12x24_data[];

#endif