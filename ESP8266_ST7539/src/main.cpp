#include "ST7539.h"

// 定义复位引脚（根据实际连接修改）
#define LCD_REST_PIN D3

// 创建LCD对象
ST7539 lcd(LCD_REST_PIN);

void setup() {
  // 初始化LCD
  lcd.begin();
  // 显示字符串
  lcd.displayString(0, 1, 0, "Hello World!");
  lcd.displayString(0, 3, 0, "ST7539 LCD Test");
//   lcd.displayString(0, 3, 0, "ESP8266 Arduino");
}

void loop() {
  // 主循环代码
}