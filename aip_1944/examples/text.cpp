#include <Arduino.h>
#include "aip1944.h"

// 定义引脚连接 (根据实际硬件连接修改)
const uint8_t AIP1944_DIO_PIN = 9;
const uint8_t AIP1944_CLK_PIN = 11;
const uint8_t AIP1944_STB_PIN = 13;

// 创建AIP1944实例
AIP1944 aip1944(AIP1944_CLK_PIN, AIP1944_STB_PIN, AIP1944_DIO_PIN);

void setup()
{
  Serial.begin(115200); // 串口初始化
  // 初始化AIP1944
  aip1944.begin(); // 屏幕初始化
  Serial.println("屏幕初始化---OK");

  Serial.println("AIP1944自定义字符示例");

  /*1
  aip1944.setDisplayMode(0x06,AIP1944_AUTO_ADDRESS_ADD_MODE);
  aip1944.writeData(0xc0, 0xff);
  aip1944.setBrightness(AIP1944_BRIGHTNESS_LEVEL_7);
  */

  /*2
  aip1944.clearDisplay();
   */

  /*3
  aip1944.clearFrame();
  aip1944.displayFrame();
  aip1944.setBrightness(AIP1944_BRIGHTNESS_LEVEL_7);
  */

  /*4
  aip1944.clearDisplay();
  aip1944.clearFrame();
  //ASCII字符顺序显示
  for (size_t i = 0X20; i <= 0X7E; i++)
  {
    aip1944.displayChar(0, i);
    aip1944.displayFrame();
    aip1944.setBrightness(AIP1944_BRIGHTNESS_LEVEL_7);
    delay(700);
  }

  */

  /*5
    aip1944.clearDisplay();
   aip1944.clearFrame();
   aip1944.displayString("ASDFMX");
   aip1944.displayFrame();
   aip1944.setBrightness(AIP1944_BRIGHTNESS_LEVEL_7);
  */
}
void loop()
{
  delay(1000);
}
