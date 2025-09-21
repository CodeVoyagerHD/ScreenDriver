/* @file aip1944.cpp
 * @brief AIP1944 LED驱动控制器实现文件
 * @details 实现AIP1944芯片的驱动功能，包括初始化、数据写入和显示控制
 * @version 3.0
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 */

#include "aip1944.h"
#include <Arduino.h>
#include <string.h>

/**
 * @brief 构造函数
 * @param clk_pin 时钟引脚编号
 * @param stb_pin 片选引脚编号
 * @param dio_pin 数据引脚编号
 */
AIP1944::AIP1944(uint8_t clk_pin, uint8_t stb_pin, uint8_t dio_pin)
    : _clk_pin(clk_pin), _stb_pin(stb_pin), _dio_pin(dio_pin)
{
  // 初始化显示RAM
  memset(_display_ram, 0, sizeof(_display_ram));
}

/**
 * @brief 初始化函数
 * @details 初始化GPIO引脚和芯片默认设置
 */
void AIP1944::begin()
{
  // 初始化引脚
  initPins();

  // 设置默认亮度 (等级7，最亮)
  setBrightness(AIP1944_BRIGHTNESS_LEVEL_7);

  // 清空显示
  clearDisplay();
}

/**
 * @brief 初始化引脚设置
 * @details 配置所有使用的GPIO引脚为输出模式并设置初始状态
 */
void AIP1944::initPins()
{
  pinMode(_clk_pin, OUTPUT);
  pinMode(_stb_pin, OUTPUT);
  pinMode(_dio_pin, OUTPUT);

  digitalWrite(_clk_pin, HIGH);
  digitalWrite(_stb_pin, HIGH);
  digitalWrite(_dio_pin, LOW);
}

/**
 * @brief 设置显示模式
 * @param mode 显示模式
 * @param address_mode 地址增加模式
 */
void AIP1944::setDisplayMode(uint8_t mode, uint8_t address_mode)
{
  sendCommand(mode);         // 显示模式设置
  sendCommand(address_mode); // 地址增加模式设置
}

/**
 * @brief 设置显示亮度
 * @param level 亮度级别 (AIP1944_BRIGHTNESS_LEVEL_0 到 AIP1944_BRIGHTNESS_LEVEL_7)
 */
void AIP1944::setBrightness(uint8_t level)
{
  // 确保亮度级别在有效范围内
  if (level >= AIP1944_BRIGHTNESS_LEVEL_0 && level <= AIP1944_BRIGHTNESS_LEVEL_7)
  {
    sendCommand(level);
  }
}

/**
 * @brief 清空显示RAM
 * @details 将所有显示RAM内容清零，关闭所有LED
 */
void AIP1944::clearDisplay()
{
  setDisplayMode(AIP1944_MODE_14x18, AIP1944_AUTO_ADDRESS_ADD_MODE);

  // 起始地址
  digitalWrite(_stb_pin, LOW);
  writeByte(AIP1944_ADDRESS_COMMAND); // 起始地址命令

  // 写入56个空数据(清空整个RAM)
  for (int i = 0; i < 56; i++)
  {
    writeByte(0x00);
  }

  digitalWrite(_stb_pin, HIGH);
}

/**
 * @brief 向指定地址写入数据
 * @param address 显示地址 (0x00-0xFF)
 * @param data 要写入的数据
 */
void AIP1944::writeData(uint8_t address, uint8_t data)
{
  digitalWrite(_stb_pin, LOW);
  writeByte(address); // 写入地址
  writeByte(data);    // 写入数据
  digitalWrite(_stb_pin, HIGH);
}

/**
 * @brief 连续写入多个数据
 * @param start_address 起始地址
 * @param data 数据数组指针
 * @param length 数据长度
 */
void AIP1944::writeContinuousData(uint8_t start_address, const uint8_t *data, uint8_t length)
{
  setDisplayMode(AIP1944_MODE_14x18, AIP1944_AUTO_ADDRESS_ADD_MODE);
  digitalWrite(_stb_pin, LOW);
  writeByte(start_address); // 写入起始地址

  // 连续写入数据
  for (int i = 0; i < length; i++)
  {
    writeByte(data[i]);
  }

  digitalWrite(_stb_pin, HIGH);
}

/**
 * @brief 发送命令到芯片
 * @param command 命令字节
 */
void AIP1944::sendCommand(uint8_t command)
{
  digitalWrite(_stb_pin, LOW);
  writeByte(command);
  digitalWrite(_stb_pin, HIGH);
}

/**
 * @brief 写入一个字节到芯片
 * @param data 要写入的字节
 */
void AIP1944::writeByte(uint8_t data)
{
  // 逐位写入数据，LSB优先
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(_clk_pin, LOW);
    delayUs(1);

    // 设置数据线电平
    digitalWrite(_dio_pin, (data & 0x01) ? HIGH : LOW);

    delayUs(1);
    digitalWrite(_clk_pin, HIGH);
    delayUs(1);

    data >>= 1; // 移位到下一位
  }
}

/**
 * @brief 微秒级延时函数
 * @param us 微秒数
 */
void AIP1944::delayUs(unsigned int us)
{
  // 基于ESP32-S2的高精度延时
  for (unsigned int i = 0; i < us; i++)
  {
    for (int j = 0; j < 40; j++)
    {
      asm volatile("nop");
    }
  }
}

/**
 * @brief 设置显存中的一字节数据
 * @param page 页地址 (0-3)
 * @param column 列地址 (0-6)
 * @param data 数据
 */
void AIP1944::setByte(uint8_t page, uint8_t column, uint8_t data)
{
  if (isValidPage(page) && isValidColumn(column))
  {
    _display_ram[page][column] = data;
  }
}

/**
 * @brief 设置显存中一字节数据的指定位
 * @param page 页地址 (0-3)
 * @param column 列地址 (0-6)
 * @param data 数据
 * @param start 起始位 (0-7)
 * @param end 结束位 (0-7)
 */
void AIP1944::setByteBits(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end)
{
  if (!isValidPage(page) || !isValidColumn(column) || start > 7 || end > 7 || start > end)
  {
    return;
  }

  uint8_t mask = 0;
  for (uint8_t i = start; i <= end; i++)
  {
    mask |= (1 << i);
  }

  _display_ram[page][column] = (_display_ram[page][column] & ~mask) | (data & mask);
}

/**
 * @brief 清空显存，准备绘制新的一帧
 */
void AIP1944::clearFrame()
{
  memset(_display_ram, 0x00, sizeof(_display_ram));
}

/**
 * @brief 将当前显存内容显示到屏幕上
 * @attention 使用固定地址模式发送显存
 */
void AIP1944::displayFrame()
{
  setDisplayMode(AIP1944_MODE_14x18, AIP1944_FIXED_ADDRESS_MODE);

  // 发送显存数据
  for (uint8_t line = 0; line < 7; line++)
  {
    writeData(0XC0 + 4 * line, _display_ram[0][line]);
  }

  for (uint8_t line = 0; line < 7; line++)
  {
    writeData(0XC1 + 4 * line, _display_ram[1][line]);
  }

  for (uint8_t line = 0; line < 7; line++)
  {
    writeData(0XC2 + 4 * line, _display_ram[2][line] & 0x01);
  }

  for (uint8_t line = 0; line < 7; line++)
  {
    writeData(0XDC + 4 * line, _display_ram[2][line] >> 1 & 0x7F | _display_ram[3][line] << 7 & 0x80);
  }

  for (uint8_t line = 0; line < 7; line++)
  {
    writeData(0XDD + 4 * line, _display_ram[3][line] >> 1 & 0x7F);
  }
}

/**
 * @brief 设置单个像素点的状态
 * @param x X坐标 (0-31)
 * @param y Y坐标 (0-6)
 * @param state 像素状态 (true-开, false-关)
 * @return 成功返回true，失败返回false
 */
bool AIP1944::setPixel(uint8_t x, uint8_t y, bool state)
{
  // 验证坐标是否在有效范围内
  if (x >= AIP1944_COLUMNS || y >= AIP1944_ROWS) // Y坐标应该是0-6，共行
  {
    return false;
  }

  // 计算页地址和列偏移
  uint8_t page = x / 8;   // 每页8列，计算所在页(0-3)
  uint8_t column = x % 8; // 计算在页内的列偏移(0-7)
  uint8_t rows = y;       // 计算所在行(0-6)
  // 确保页和列在有效范围内
  if (!isValidPage(page) || column >= 8)
  {
    return false;
  }

  // 设置或清除特定位
  if (state)
  {
    // 设置位为1
    _display_ram[page][rows] |= (1 << column);
  }
  else
  {
    // 设置位为0
    _display_ram[page][rows] &= ~(1 << column);
  }

  return true;
}
/**
 * @brief 在指定位置绘制一个字符（优化版，无缩放）
 * @param x 起始X坐标 (0-31)
 * @param y 起始Y坐标 (0-6)
 * @param character 要显示的字符
 * @param font 字体定义指针
 * @return 成功返回true，失败返回false
 */
bool AIP1944::drawChar(uint8_t x, uint8_t y, char character, const FontDef *font)
{
  // 确保字符在可显示范围内 (32-126)
  if (character < 0x20 || character > 0x7E)
  {
    return false;
  }

  // 获取字符索引
  uint8_t char_index = character - 32;

  // 检查坐标是否有效
  if (x >= AIP1944_COLUMNS || y >= AIP1944_ROWS)
  {
    return false;
  }

  // 检查字符是否会超出屏幕边界
  if (x + font->width > AIP1944_COLUMNS ||
      y + font->height > AIP1944_ROWS)
  {
    return false;
  }

  // 遍历字符点阵数据的每一行
  for (uint8_t row = 0; row < font->height; row++)
  {
    uint8_t row_data = font->data[char_index][row];

    // 遍历每一列
    for (uint8_t col = 0; col < font->width; col++)
    {
      // 检查当前位是否为1（点亮像素）
      // 使用掩码提取特定位
      // bool pixel_state = (row_data & (1 << (font->width - 1 - col))) != 0;
      bool pixel_state = (row_data & (1 << ( col))) != 0;//低位在前

      // 设置单个像素
      setPixel(x + col, y + row, pixel_state);
    }
  }

  return true;
} /**
   * @brief 在指定位置绘制字符串（优化版，无缩放）
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param str 要显示的字符串
   * @param font 字体定义指针
   * @param spacing 字符间距 (默认为1)
   * @return 成功返回true，失败返回false
   */
bool AIP1944::drawString(uint8_t x, uint8_t y, const char *str, const FontDef *font, uint8_t spacing)
{
  uint8_t current_x = x;
  uint8_t char_width = font->width + spacing;

  for (uint8_t i = 0; str[i] != '\0'; i++)
  {
    // 检查是否超出屏幕范围
    if (current_x >= AIP1944_COLUMNS)
    {
      break;
    }

    // 绘制单个字符
    if (!drawChar(current_x, y, str[i], font))
    {
      return false;
    }

    // 移动到下一个字符位置
    current_x += char_width;
  }

  return true;
}
/**
 * @brief 绘制一条水平线
 * @param x 起始X坐标 (0-31)
 * @param y Y坐标 (0-6)
 * @param length 线长度
 * @param state 线状态 (true-实线, false-虚线)
 */
void AIP1944::drawHLine(uint8_t x, uint8_t y, uint8_t length, bool state)
{
  for (uint8_t i = 0; i < length; i++)
  {
    if (x + i < AIP1944_COLUMNS)
    {
      setPixel(x + i, y, state);
    }
  }
}

/**
 * @brief 绘制一条垂直线
 * @param x X坐标 (0-31)
 * @param y 起始Y坐标 (0-6)
 * @param length 线长度
 * @param state 线状态 (true-实线, false-虚线)
 */
void AIP1944::drawVLine(uint8_t x, uint8_t y, uint8_t length, bool state)
{
  for (uint8_t i = 0; i < length; i++)
  {
    if (y + i < AIP1944_ROWS)
    {
      setPixel(x, y + i, state);
    }
  }
}

/**
 * @brief 绘制矩形
 * @param x 起始X坐标 (0-31)
 * @param y 起始Y坐标 (0-6)
 * @param width 矩形宽度
 * @param height 矩形高度
 * @param filled 是否填充
 */
void AIP1944::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled)
{
  if (filled)
  {
    // 绘制填充矩形
    for (uint8_t i = 0; i < height; i++)
    {
      drawHLine(x, y + i, width, true);
    }
  }
  else
  {
    // 绘制空心矩形
    drawHLine(x, y, width, true);              // 上边
    drawHLine(x, y + height - 1, width, true); // 下边
    drawVLine(x, y, height, true);             // 左边
    drawVLine(x + width - 1, y, height, true); // 右边
  }
}

/**
 * @brief 绘制位图
 * @param x 起始X坐标 (0-31)
 * @param y 起始Y坐标 (0-6)
 * @param bitmap 位图数据数组
 * @param width 位图宽度
 * @param height 位图高度
 */
void AIP1944::drawBitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height)
{
  for (uint8_t row = 0; row < height; row++)
  {
    for (uint8_t col = 0; col < width; col++)
    {
      // 计算位图数据中的字节和位位置
      uint8_t byte_index = row * ((width + 7) / 8) + col / 8;
      uint8_t bit_index = 7 - (col % 8);

      // 获取像素状态
      bool pixel_state = (bitmap[byte_index] & (1 << bit_index)) != 0;

      // 绘制像素
      if (x + col < AIP1944_COLUMNS && y + row < AIP1944_ROWS)
      {
        setPixel(x + col, y + row, pixel_state);
      }
    }
  }
}

/**
 * @brief 绘制进度条
 * @param x 起始X坐标 (0-31)
 * @param y 起始Y坐标 (0-6)
 * @param width 进度条宽度
 * @param height 进度条高度
 * @param progress 进度值 (0-100)
 */
void AIP1944::drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t progress)
{
  // 绘制边框
  drawRect(x, y, width, height, false);

  // 计算填充宽度
  uint8_t fill_width = (width - 2) * progress / 100;

  // 绘制填充部分
  if (fill_width > 0)
  {
    for (uint8_t i = 1; i < height - 1; i++)
    {
      drawHLine(x + 1, y + i, fill_width, true);
    }
  }
}

/**
 * @brief 绘制自定义符号
 * @param x 起始X坐标 (0-31)
 * @param y 起始Y坐标 (0-6)
 * @param symbol_data 符号数据数组
 * @param width 符号宽度
 * @param height 符号高度
 */
void AIP1944::drawSymbol(uint8_t x, uint8_t y, const uint8_t *symbol_data, uint8_t width, uint8_t height)
{
  for (uint8_t row = 0; row < height; row++)
  {
    for (uint8_t col = 0; col < width; col++)
    {
      // 计算数据中的位位置
      uint8_t bit_mask = 1 << (7 - col % 8);
      uint8_t byte_index = row * ((width + 7) / 8) + col / 8;

      // 获取像素状态
      bool pixel_state = (symbol_data[byte_index] & bit_mask) != 0;

      // 绘制像素
      if (x + col < AIP1944_COLUMNS && y + row < AIP1944_ROWS)
      {
        setPixel(x + col, y + row, pixel_state);
      }
    }
  }
}
/**
 * @brief 在指定位置显示ASCII字符（优化版）
 * @param position 显示位置 (0-5)
 * @param character 要显示的字符
 * @param font 字体定义指针
 * @return 成功返回true，失败返回false
 */
bool AIP1944::displayChar(uint8_t position, char character, const FontDef *font)
{
  // 确保字符在可显示范围内 (32-126)
  if (character < 0x20 || character > 0x7E)
  {
    return false;
  }

  // 确保位置有效
  if (position > 5)
  {
    return false;
  }

  // 获取字符索引
  uint8_t char_index = character - 32;

  // 根据位置设置字符数据
  // 这里假设使用5x7字体，因为这是displayChar原本的设计
  for (uint8_t row = 0; row < font->height; row++)
  {
    uint8_t row_data = font->data[char_index][row];

    switch (position)
    {
    case 0:
      setByteBits(0, row, row_data, 0, 4);
      break;
    case 1:
      setByteBits(0, row, row_data << 5, 5, 7);
      setByteBits(1, row, row_data >> 3, 0, 1);
      break;
    case 2:
      setByteBits(1, row, row_data << 4, 4, 7);
      setByteBits(2, row, row_data >> 4, 0, 0);
      break;
    case 3:
      setByteBits(2, row, row_data << 1, 1, 5);
      break;
    case 4:
      setByteBits(2, row, row_data << 6, 6, 7);
      setByteBits(3, row, row_data >> 2, 0, 2);
      break;
    case 5:
      setByteBits(3, row, row_data << 3, 3, 7);
      break;
    default:
      return false;
    }
  }

  return true;
}
/**
 * @brief 显示字符串
 * @param str 要显示的字符串
 * @return 成功返回true，失败返回false
 * @attention 使用直接设置显存实现
 */
bool AIP1944::displayString(const char *str)
{
  uint8_t position = 0;

  for (uint8_t i = 0; str[i] != '\0' && position <= 5; i++)
  {
    if (!displayChar(position, str[i], &Font_5x7))
    {
      return false;
    }
    position++;
  }

  return true;
}

/**
 * @brief 显示符号
 * @param symbol_data 符号数据数组
 * @attention 使用直接设置显存实现
 */
void AIP1944::displaySymbol(const uint8_t symbol_data[7])
{
  for (uint8_t i = 0; i < 7; i++)
  {
    setByteBits(1, i, symbol_data[i], 2, 3);
  }
}

/**
 * @brief 验证位置是否有效
 * @param position 位置值0-5
 * @return 有效返回true，无效返回false
 */
bool AIP1944::isValidPosition(uint8_t position)
{
  return position <= 5;
}

/**
 * @brief 验证页地址是否有效
 * @param page 页地址0-4
 * @return 有效返回true，无效返回false
 */
bool AIP1944::isValidPage(uint8_t page)
{
  return page < AIP1944_PAGES;
}

/**
 * @brief 验证列地址是否有效
 * @param column 列地址0-32
 * @return 有效返回true，无效返回false
 */
bool AIP1944::isValidColumn(uint8_t column)
{
  return column < AIP1944_COLUMNS;
}
