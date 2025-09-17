#include "ST7539.h"
#include "font_8x16.h"

ST7539::ST7539(uint8_t restPin, uint8_t addrCmd, uint8_t addrData) {
  _restPin = restPin;
  _addrCmd = addrCmd;
  _addrData = addrData;
}

void ST7539::begin() {
  pinMode(_restPin, OUTPUT);
  Wire.begin();
  
  // 初始化LCD指令序列
  digitalWrite(_restPin, LOW);
  delay(20);
  digitalWrite(_restPin, HIGH);
  delay(500);
  
  sendCommand(0xE2); // 软复位
  sendCommand(0xA3); // 刷新率
  sendCommand(0xEB); // 设置偏压比
  sendCommand(0xC2); // 方向
  setContrast(0x2F); // 设置对比度
  sendCommand(0xB0); // 显示起始行
  sendCommand(0x10); 
  sendCommand(0x00); 
  sendCommand(0x40); 
  sendCommand(0xAF); // 显示开
  delay(100);
  
  // 清屏
  clear();
}

void ST7539::sendCommand(uint8_t command) {
  Wire.beginTransmission(_addrCmd);
  Wire.write(command);
  Wire.endTransmission();
  delay(1);
}

void ST7539::sendData(uint8_t data) {
  Wire.beginTransmission(_addrData);
  Wire.write(data);
  Wire.endTransmission();
  delay(1);
}

void ST7539::setAddress(uint8_t page, uint8_t column) {
  uint8_t Page = page - 1;
  sendCommand(0xB0 + Page);
  sendCommand(((column >> 4) & 0x0F) + 0x10); // 列地址MSB
  sendCommand(column & 0x0F); // 列地址LSB
}

void ST7539::displayString(uint8_t reverse, uint8_t page, uint8_t column, const char* str) {
  uint16_t i = 0, j = 0, k = 0;
  
  while(str[i] != '\0') {
    if(str[i] >= 0x20 && str[i] <= 0x7E) {
      j = str[i] - 0x20;
      setAddress(page, column);
      
      for(k = 0; k < 8; k++) {
        if(reverse) {
          sendData(Ascii_8x16[j][k]);
        } else {
          sendData(~Ascii_8x16[j][k]);
        }
      }
      
      setAddress(page + 1, column);
      
      for(k = 0; k < 8; k++) {
        if(reverse) {
          sendData(Ascii_8x16[j][k + 8]);
        } else {
          sendData(~Ascii_8x16[j][k + 8]);
        }
      }
      
      i++;
      column += 8;
    }
  }
}

void ST7539::clear() {
  for(uint8_t page = 1; page <= 4; page++) {
    setAddress(page, 0);
    for(uint8_t col = 0; col < 128; col++) {
      sendData(0x00);
    }
  }
}

void ST7539::setContrast(uint8_t contrast) {
  sendCommand(0x81); // 对比度命令
  sendCommand(contrast); // 对比度值
}