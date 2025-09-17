
// 示例代码
#include "display_graphics.h"
#include "display_char.h"
#include <Arduino.h>
void setup()
{
    // 初始化屏幕
    display_for_laowang_init();

    // 清屏
    clear_screen(0);

    // 显示字符串
    display_string(0, 0, "ABCab12", FONT_SIZE_8x16, MODE_NORMAL, 1);
    display_string(0, 16, "AB2", FONT_SIZE_6x8, MODE_NORMAL, 1);

}

void loop()
{
    // 主循环
}
