/**
 * @file aip1944.h
 * @brief AIP1944 LED驱动控制器头文件
 * @details 提供AIP1944芯片的驱动接口，支持16段16位或24段8位LED显示
 * @version 3.0
 * @date 2025-09-20
 *
 * @copyright Copyright (c) 2025
 */

#ifndef AIP1944_H
#define AIP1944_H

#include <Arduino.h>
#include "font.h"
// 指令定义
#define AIP1944_DISPLAY_MODE 0x00            // 显示模式设置
#define AIP1944_DATA_COMMAND_MODE 0x40       // 数据命令模式设置
#define AIP1944_DISPLAY_CONTROL_COMMAND 0x80 // 显示控制命令设置
#define AIP1944_ADDRESS_COMMAND 0xC0         // 地址命令设置

// 显示模式配置
#define AIP1944_MODE_8x24 0x00  // 8位14段模式
#define AIP1944_MODE_9x23 0x01  // 9位23段模式
#define AIP1944_MODE_10x22 0x02 // 10位22段模式
#define AIP1944_MODE_11x21 0x03 // 11位21段模式
#define AIP1944_MODE_12x20 0x04 // 12位20段模式
#define AIP1944_MODE_13x19 0x05 // 13位19段模式
#define AIP1944_MODE_14x18 0x06 // 14位18段模式
#define AIP1944_MODE_15x17 0x07 // 15位17段模式
#define AIP1944_MODE_16x16 0x08 // 16位16段模式

// 数据设置
#define AIP1944_WRITE_DATA_MODE (AIP1944_DATA_COMMAND_MODE | 0x00)         // 写数据到显示寄存器
#define AIP1944_READ_KEY_SCAN_DATA_MODE (AIP1944_DATA_COMMAND_MODE | 0x20) // 读按键扫数据
#define AIP1944_AUTO_ADDRESS_ADD_MODE (AIP1944_DATA_COMMAND_MODE | 0x00)   // 地址自动加一
#define AIP1944_FIXED_ADDRESS_MODE (AIP1944_DATA_COMMAND_MODE | 0x04)      // 固定地址
#define AIP1944_NORMAL_MODE (AIP1944_DATA_COMMAND_MODE | 0x00)             // 普通模式
#define AIP1944_TEST_MODE (AIP1944_DATA_COMMAND_MODE | 0x80)               // 测试模式

// 显示控制亮度等级配置 (0-7级，0最暗，7最亮)
#define AIP1944_DISPLAY_OFF 0x00
#define AIP1944_DISPLAY_ON 0x08
#define AIP1944_BRIGHTNESS_LEVEL_0 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x00)
#define AIP1944_BRIGHTNESS_LEVEL_1 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x01)
#define AIP1944_BRIGHTNESS_LEVEL_2 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x02)
#define AIP1944_BRIGHTNESS_LEVEL_3 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x03)
#define AIP1944_BRIGHTNESS_LEVEL_4 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x04)
#define AIP1944_BRIGHTNESS_LEVEL_5 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x05)
#define AIP1944_BRIGHTNESS_LEVEL_6 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x06)
#define AIP1944_BRIGHTNESS_LEVEL_7 (AIP1944_DISPLAY_CONTROL_COMMAND | AIP1944_DISPLAY_ON | 0x07)

// 显示参数
#define AIP1944_COLUMNS 32                // AIP1944列数
#define AIP1944_ROWS 7                    // AIP1944行数
#define AIP1944_PAGES AIP1944_COLUMNS / 8 // AIP1944页数 (32/8 )4

/**
 * @brief AIP1944驱动类
 * @details 提供AIP1944芯片的完整控制功能，包括显示控制、亮度调节等
 */
class AIP1944
{
public:
  /**
   * @brief 构造函数
   * @param clk_pin 时钟引脚编号
   * @param stb_pin 片选引脚编号
   * @param dio_pin 数据引脚编号
   */
  AIP1944(uint8_t clk_pin, uint8_t stb_pin, uint8_t dio_pin);

  /**
   * @brief 初始化函数
   * @details 初始化GPIO引脚和芯片默认设置
   */
  void begin();

  /**
   * @brief 设置显示模式
   * @param mode 显示模式
   * @param address_mode 地址增加模式
   */
  void setDisplayMode(uint8_t mode, uint8_t address_mode);

  /**
   * @brief 设置显示亮度
   * @param level 亮度级别 (AIP1944_BRIGHTNESS_LEVEL_0 到 AIP1944_BRIGHTNESS_LEVEL_7)
   */
  void setBrightness(uint8_t level);

  /**
   * @brief 清空显示RAM
   * @details 将所有显示RAM内容清零，关闭所有LED
   */
  void clearDisplay();

  /**
   * @brief 向指定地址写入数据
   * @param address 显示地址 (0x00-0xFF)
   * @param data 要写入的数据
   */
  void writeData(uint8_t address, uint8_t data);

  /**
   * @brief 连续写入多个数据
   * @param start_address 起始地址
   * @param data 数据数组指针
   * @param length 数据长度
   */
  void writeContinuousData(uint8_t start_address, const uint8_t *data, uint8_t length);

  /**
   * @brief 发送命令到芯片
   * @param command 命令字节
   */
  void sendCommand(uint8_t command);

  /**
   * @brief 设置显存中的一字节数据
   * @param page 页地址 (0-4)
   * @param column 列地址 (0-6)
   * @param data 数据
   */
  void setByte(uint8_t page, uint8_t column, uint8_t data);

  /**
   * @brief 设置显存中一字节数据的指定位
   * @param page 页地址 (0-4)
   * @param column 列地址 (0-6)
   * @param data 数据
   * @param start 起始位 (0-7)
   * @param end 结束位 (0-7)
   */
  void setByteBits(uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end);

  /**
   * @brief 清空显存，准备绘制新的一帧
   */
  void clearFrame();

  /**
   * @brief 将当前显存内容显示到屏幕上
   */
  void displayFrame();
  /**
   * @brief 设置单个像素点的状态
   * @param x X坐标 (0-31)
   * @param y Y坐标 (0-7)
   * @param state 像素状态 (true-开, false-关)
   * @return 成功返回true，失败返回false
   */
  bool setPixel(uint8_t x, uint8_t y, bool state);
  /**
   * @brief 在指定位置绘制一个字符（优化版，无缩放）
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param character 要显示的字符
   * @param font 字体定义指针
   * @return 成功返回true，失败返回false
   */
  bool drawChar(uint8_t x, uint8_t y, char character, const FontDef *font);
  /**
   * @brief 在指定位置绘制字符串（优化版，无缩放）
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param str 要显示的字符串
   * @param font 字体定义指针
   * @param spacing 字符间距 (默认为1)
   * @return 成功返回true，失败返回false
   */
  bool drawString(uint8_t x, uint8_t y, const char *str, const FontDef *font, uint8_t spacing);
  /**
   * @brief 绘制一条水平线
   * @param x 起始X坐标 (0-31)
   * @param y Y坐标 (0-6)
   * @param length 线长度
   * @param state 线状态 (true-实线, false-虚线)
   */
  void drawHLine(uint8_t x, uint8_t y, uint8_t length, bool state);
  /**
   * @brief 绘制一条垂直线
   * @param x X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param length 线长度
   * @param state 线状态 (true-实线, false-虚线)
   */
  void drawVLine(uint8_t x, uint8_t y, uint8_t length, bool state);
  /**
   * @brief 绘制矩形
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param width 矩形宽度
   * @param height 矩形高度
   * @param filled 是否填充
   */
  void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool filled);
  /**
   * @brief 绘制位图
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param bitmap 位图数据数组
   * @param width 位图宽度
   * @param height 位图高度
   */
  void drawBitmap(uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height);
  /**
   * @brief 绘制进度条
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param width 进度条宽度
   * @param height 进度条高度
   * @param progress 进度值 (0-100)
   */
  void drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t progress);

  /**
   * @brief 绘制自定义符号
   * @param x 起始X坐标 (0-31)
   * @param y 起始Y坐标 (0-6)
   * @param symbol_data 符号数据数组
   * @param width 符号宽度
   * @param height 符号高度
   */
  void drawSymbol(uint8_t x, uint8_t y, const uint8_t *symbol_data, uint8_t width, uint8_t height);
  /**
   * @brief 在指定位置显示ASCII字符（优化版）
   * @param position 显示位置 (0-5)
   * @param character 要显示的字符
   * @param font 字体定义指针
   * @return 成功返回true，失败返回false
   */
  bool displayChar(uint8_t position, char character, const FontDef *font);

  /**
   * @brief 显示字符串
   * @param str 要显示的字符串
   * @return 成功返回true，失败返回false
   */
  bool displayString(const char *str);

  /**
   * @brief 显示符号
   * @param symbol_data 符号数据数组
   */
  void displaySymbol(const uint8_t symbol_data[7]);

private:
  uint8_t _clk_pin;                                  // 时钟引脚
  uint8_t _stb_pin;                                  // 片选引脚
  uint8_t _dio_pin;                                  // 数据引脚
  uint8_t _display_ram[AIP1944_PAGES][AIP1944_ROWS]; // 显示RAM

  /**
   * @brief 初始化引脚设置
   */
  void initPins();

  /**
   * @brief 写入一个字节到芯片
   * @param data 要写入的字节
   */
  void writeByte(uint8_t data);

  /**
   * @brief 微秒级延时函数
   * @param us 微秒数
   */
  void delayUs(unsigned int us);

  /**
   * @brief 验证位置是否有效
   * @param position 位置值
   * @return 有效返回true，无效返回false
   */
  bool isValidPosition(uint8_t position);

  /**
   * @brief 验证页地址是否有效
   * @param page 页地址
   * @return 有效返回true，无效返回false
   */
  bool isValidPage(uint8_t page);

  /**
   * @brief 验证列地址是否有效
   * @param column 列地址
   * @return 有效返回true，无效返回false
   */
  bool isValidColumn(uint8_t column);
};

#endif // AIP1944_H