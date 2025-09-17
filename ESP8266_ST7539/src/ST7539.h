#ifndef ST7539_H
#define ST7539_H

#include <Arduino.h>
#include <Wire.h>

// 默认I2C地址
#define LCD_I2C_ADDR_CMD 0x3E
#define LCD_I2C_ADDR_DATA 0x3F

class ST7539 {
public:
  // 构造函数
  ST7539(uint8_t restPin, uint8_t addrCmd = LCD_I2C_ADDR_CMD, uint8_t addrData = LCD_I2C_ADDR_DATA);
  
  // 初始化函数
  void begin();
  
  // 发送命令
  void sendCommand(uint8_t command);
  
  // 发送数据
  void sendData(uint8_t data);
  
  // 设置显示地址
  void setAddress(uint8_t page, uint8_t column);
  
  // 显示字符串
  void displayString(uint8_t reverse, uint8_t page, uint8_t column, const char* str);
  
  // 清屏
  void clear();
  
  // 设置对比度
  void setContrast(uint8_t contrast);
  
private:
  uint8_t _restPin;
  uint8_t _addrCmd;
  uint8_t _addrData;
};

#endif