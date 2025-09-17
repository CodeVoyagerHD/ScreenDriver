#ifndef DISPLAY_FOR_LAOWANG_H
#define DISPLAY_FOR_LAOWANG_H

#include "display_ist3931.h"

// 屏幕尺寸定义
#define HEIGHT_PIX 32  // 屏幕高度(像素)
#define WIDTH_PIX 64   // 屏幕宽度(像素)

uint8_t zxc_i2c_write_only(uint8_t device_addr, uint8_t* data, uint16_t len);
/**
 * @brief 毫秒延时函数实现
 * @param ms 延时毫秒数
 */
void zxc_delay_ms(uint16_t ms);
// 初始化函数
uint8_t display_for_laowang_init();

// 清屏函数
void clear_screen(uint8_t val);

// 像素写入函数
uint8_t screen_write_by_pix(const uint8_t x, const uint8_t y,
                            uint8_t width, uint8_t height, const void *buf);

#endif