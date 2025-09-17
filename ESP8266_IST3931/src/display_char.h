// display_char.h
#ifndef DISPLAY_CHAR_H
#define DISPLAY_CHAR_H

#include "display_font.h"
#include "display_for_laowang.h"

/**
 * @brief 在指定位置显示一个字符
 * @param x 起始X坐标(像素)
 * @param y 起始Y坐标(像素)
 * @param c 要显示的字符
 * @param font 字体大小
 * @param mode 显示模式
 * @return 0:成功, 1:失败
 */
uint8_t display_char(uint8_t x, uint8_t y, char c, font_size_t font, char_display_mode_t mode);

/**
 * @brief 在指定位置显示字符串
 * @param x 起始X坐标(像素)
 * @param y 起始Y坐标(像素)
 * @param str 要显示的字符串
 * @param font 字体大小
 * @param mode 显示模式
 * @param spacing 字符间距(像素)
 * @return 0:成功, 1:失败
 */
uint8_t display_string(uint8_t x, uint8_t y, const char* str, font_size_t font, 
                      char_display_mode_t mode, uint8_t spacing);


#endif