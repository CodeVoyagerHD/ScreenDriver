// display_char.cpp
#include "display_char.h"
#include <string.h>
#include <stdio.h>
/**
 * @brief 在指定位置显示一个字符
 * @param x 起始X坐标(像素)
 * @param y 起始Y坐标(像素)
 * @param c 要显示的字符
 * @param font 字体大小
 * @param mode 显示模式
 * @return 0:成功, 1:失败
 */
uint8_t display_char(uint8_t x, uint8_t y, char c, font_size_t font, char_display_mode_t mode)
{
    // 获取字体
    const font_t *font_ptr = get_font(font);

    // 只支持ASCII 32-126
    if (c < 32 || c > 126)
    {
        c = ' '; // 显示空格代替不支持字符
    }

    // 计算字符在字模数组中的偏移量
    uint16_t char_offset = (c - 32) * font_ptr->bytes_per_char;

    // 检查坐标是否超出屏幕范围
    if (x + font_ptr->width > WIDTH_PIX || y + font_ptr->height > HEIGHT_PIX)
    {
        return 1;
    }

    // 处理不同的显示模式
    if (mode == MODE_NORMAL || mode == MODE_OVERWRITE)
    {
        // 直接写入模式 - 使用屏幕缓冲区
        screen_write_by_pix(x, y, font_ptr->width, font_ptr->height,
                            &font_ptr->data[char_offset]);
    }
    else
    {
        // 特殊模式需要逐像素处理
        uint8_t temp_buf[font_ptr->height][(font_ptr->width + 7) / 8];
        memset(temp_buf, 0, sizeof(temp_buf));

        // 从屏幕读取当前内容
        // 注意: 这里需要实现一个屏幕读取函数，但IST3931通常不支持读取
        // 作为替代，我们可以使用屏幕缓冲区中的数据

        // 复制字模数据到临时缓冲区
        for (uint8_t row = 0; row < font_ptr->height; row++)
        {
            for (uint8_t col = 0; col < font_ptr->width; col++)
            {
                uint8_t byte_idx = col / 8;
                uint8_t bit_idx = col % 8;

                // 获取字模中的像素值
                uint8_t font_byte = font_ptr->data[char_offset + row * ((font_ptr->width + 7) / 8) + byte_idx];
                uint8_t font_pixel = (font_byte >> (7 - bit_idx)) & 0x01;

                // 根据模式处理像素
                uint8_t screen_pixel = 0; // 假设屏幕当前为0

                switch (mode)
                {
                case MODE_INVERT:
                    screen_pixel = !font_pixel;
                    break;
                case MODE_XOR:
                    screen_pixel = screen_pixel ^ font_pixel;
                    break;
                default:
                    screen_pixel = font_pixel;
                    break;
                }

                // 设置临时缓冲区中的像素
                if (screen_pixel)
                {
                    temp_buf[row][byte_idx] |= (1 << (7 - bit_idx));
                }
            }
        }

        // 写入处理后的数据
        screen_write_by_pix(x, y, font_ptr->width, font_ptr->height, temp_buf);
    }

    return 0;
}

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
uint8_t display_string(uint8_t x, uint8_t y, const char *str, font_size_t font,
                       char_display_mode_t mode, uint8_t spacing)
{
    const font_t *font_ptr = get_font(font);
    uint8_t current_x = x;

    for (uint32_t i = 0; str[i] != '\0'; i++)
    {
        // 检查是否超出屏幕右边界
        if (current_x + font_ptr->width > WIDTH_PIX)
        {
            break; // 超出屏幕，停止显示
        }

        // 显示当前字符
        display_char(current_x, y, str[i], font, mode);

        // 移动到下一个字符位置
        current_x += font_ptr->width + spacing;
    }

    return 0;
}

