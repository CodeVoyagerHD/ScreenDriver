#include "display_for_laowang.h"
#include <string.h>
#include <Wire.h>

// 屏幕缓冲区 (32行 x 8字节，每字节8像素)
static uint8_t screen_buf[HEIGHT_PIX][WIDTH_PIX / 8] = {{0}};

// 默认配置
static const struct ist3931_config config = {
  .type = LAOWANG,      // 屏幕类型
  .vc = 1,              // 电压转换电路使能
  .vf = 1,              // 电压跟随电路使能
  .bias = 2,            // 偏置比
  .ct = 150,            // 对比度设置
  .duty = 32,           // 扫描占空比
  .fr = 60,             // 帧频分频比
  .shl = 1,             // 从 COM1->COMN
  .adc = 0,             // 从 SEG1->SEG132
  .eon = 0,             // 正常显示
  .rev = 0,             // RAM中1映射到LCD点亮
  .x_offset = 0,        // 水平偏移
  .y_offset = 0,        // 垂直偏移
  .i2c_write = zxc_i2c_write_only, // I2C写入函数(需用户实现)
  .delay = zxc_delay_ms            // 延时函数(需用户实现)
};

/**
 * @brief I2C写入函数实现(用户需根据实际硬件实现)
 * @param device_addr 设备地址
 * @param data 数据指针
 * @param len 数据长度
 * @return 0:成功, 1:失败
 */
uint8_t zxc_i2c_write_only(uint8_t device_addr, uint8_t* data, uint16_t len) {
  Wire.beginTransmission(device_addr);
  Wire.write(data, len);
  return Wire.endTransmission() == 0 ? 0 : 1;
}

/**
 * @brief 毫秒延时函数实现
 * @param ms 延时毫秒数
 */
void zxc_delay_ms(uint16_t ms) {
  delay(ms);
}

/**
 * @brief 初始化显示
 * @return 0:成功, 1:失败
 */
uint8_t display_for_laowang_init() {
  // 初始化I2C
  Wire.begin();
  
  // 初始化IST3931控制器
  if (ist3931_init(&config)) {
    return 1;
  }
  
  // 清屏
  clear_screen(0);
  return 0;
}

/**
 * @brief 清屏
 * @param val 填充值(0或1)
 */
void clear_screen(uint8_t val) {
  // 将0/1转换为0/0xFF
  uint8_t fill_val = (val == 0) ? 0 : 0xFF;
  
  // 填充屏幕缓冲区
  memset(screen_buf, fill_val, HEIGHT_PIX * (WIDTH_PIX / 8));
  
  // 将缓冲区内容写入屏幕
  screen_write_by_pix(0, 0, WIDTH_PIX, HEIGHT_PIX, screen_buf);
}

/**
 * @brief 按像素位置和宽度写入数据
 * @param x 水平起始坐标
 * @param y 垂直起始坐标
 * @param width 宽度(像素)
 * @param height 高度(像素)
 * @param buf 数据缓冲区
 * @return 0:成功, 1:失败
 */
uint8_t screen_write_by_pix(const uint8_t x, const uint8_t y,
                            uint8_t width, uint8_t height, const void *buf) {
  // 检查边界
  if ((x + width) > WIDTH_PIX || (y + height) > HEIGHT_PIX) {
    return 1; // 超出范围
  }

  uint8_t *buf_pointer = (uint8_t *)buf;
  uint8_t x_start = x / 8;      // 起始字节坐标
  uint8_t x_bits = x % 8;       // 起始位偏移
  uint8_t x_end = (x + width - 1) / 8; // 结束字节坐标

  // 逐行处理
  for (uint8_t i = 0; i < height; i++) {
    buf_pointer = (uint8_t *)buf + i * ((width + 7) / 8); // 重置缓冲区指针
    
    // 逐字节处理
    for (uint8_t j = x_start; j <= x_end; j++) {
      uint8_t before_b = 0;
      uint8_t current_b = 0;

      if (j == x_start) {
        // 起始字节处理：保留已有数据的高位部分
        before_b = (screen_buf[i + y][j] & ~(0xFF >> x_bits));
        current_b = ((*buf_pointer) >> x_bits); // 提取当前字节的高位到低位
        buf_pointer++;
      } else {
        // 中间字节：处理跨字节的数据
        before_b = (*(buf_pointer - 1) << (8 - x_bits)); // 提取上一个字节的低位到高位

        if (j == x_end && x_bits != 0) {
          // 结束字节且有位偏移：保留已有数据的低位部分
          current_b = (screen_buf[i + y][j] & (0xFF >> x_bits));
        } else {
          current_b = ((*buf_pointer) >> x_bits);
          buf_pointer++;
        }
      }

      // 合并数据并更新屏幕缓冲区
      screen_buf[i + y][j] = (before_b | current_b);
    }

    // 适配老王屏幕的特殊地址映射
    uint8_t ay_true = i + y;
    ay_true = (ay_true % 2 == 0) ? (ay_true / 2) : ((ay_true - 1) / 2 + 16);

    // 更新硬件显示
    ist3931_driver_set_ay(&config, ay_true);
    ist3931_driver_set_ax(&config, x_start);
    ist3931_write_bus(&config, &screen_buf[i + y][x_start], false, x_end - x_start + 1);
  }

  return 0;
}