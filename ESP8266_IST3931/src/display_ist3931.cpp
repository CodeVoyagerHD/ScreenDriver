#include "display_ist3931.h"
#include <Wire.h>  // ESP8266 Arduino I2C库

// IST3931设备地址
#define IST3931_ADDR 0x3F  // 0x7E右移一位得到7位地址

/**
 * @brief 通过I2C总线向IST3931发送数据
 * @param config 配置结构体指针
 * @param buf 数据缓冲区
 * @param command 是否为命令(true:命令, false:数据)
 * @param num_bytes 数据字节数
 * @return 0:成功, 1:失败
 */
uint8_t ist3931_write_bus(const struct ist3931_config* config, uint8_t *buf, bool command,
                         uint16_t num_bytes) {
  // 控制字节：命令或数据
  uint8_t control_byte = command ? IST3931_CMD_BYTE : IST3931_DATA_BYTE;
  
  // 准备发送缓冲区
  uint8_t i2c_write_buf[IST3931_RAM_WIDTH * 2];
  
  // 格式化数据：每个数据字节前添加控制字节
  for (uint16_t i = 0; i < num_bytes; i++) {
    i2c_write_buf[i * 2] = control_byte;
    i2c_write_buf[i * 2 + 1] = buf[i];
  };
  
  // 调用用户提供的I2C写入函数
  return config->i2c_write(IST3931_ADDR, i2c_write_buf, num_bytes * 2);
}

/**
 * @brief 设置电源配置
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static uint8_t ist3931_set_power(const struct ist3931_config* config) {
  uint8_t cmd_buf = IST3931_CMD_POWER_CONTROL | config->vc | config->vf << 1;
  return ist3931_write_bus(config, &cmd_buf, true, 1);
}

/**
 * @brief 设置偏置电压
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static inline uint8_t ist3931_set_bias(const struct ist3931_config* config) {
  uint8_t cmd_buf = IST3931_CMD_BIAS | config->bias;
  return ist3931_write_bus(config, &cmd_buf, true, 1);
}

/**
 * @brief 设置对比度
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static inline uint8_t ist3931_set_ct(const struct ist3931_config* config) {
  uint8_t cmd_buf[2] = {IST3931_CMD_CT, config->ct};
  return ist3931_write_bus(config, cmd_buf, true, 2);
}

/**
 * @brief 设置帧频
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static inline uint8_t ist3931_set_fr(const struct ist3931_config* config) {
  uint8_t cmd_buf[3] = {IST3931_CMD_FRAME_CONTROL, 
                        static_cast<uint8_t>(config->fr & 0x00FF), 
                        static_cast<uint8_t>(config->fr >> 8)};
  return ist3931_write_bus(config, cmd_buf, true, 3);
}

/**
 * @brief 设置占空比
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static uint8_t ist3931_set_duty(const struct ist3931_config* config) {
  uint8_t cmd_buf[2] = {(IST3931_CMD_SET_DUTY_LSB | (config->duty & 0x0F)),
                        (IST3931_CMD_SET_DUTY_MSB | (config->duty >> 4))};
  return ist3931_write_bus(config, cmd_buf, true, 2);
}

/**
 * @brief 设置显示控制参数
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static uint8_t ist3931_driver_display_control(const struct ist3931_config* config) {
  uint8_t cmd_buf = IST3931_CMD_DRIVER_DISPLAY_CONTROL | config->shl << 3 | config->adc << 2 |
                    config->eon << 1 | config->rev;
  return ist3931_write_bus(config, &cmd_buf, true, 1);
}

/**
 * @brief 开启显示
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static uint8_t ist3931_driver_set_display_on(const struct ist3931_config* config) {
  uint8_t cmd_buf = IST3931_CMD_DISPLAY_ON_OFF | 1;
  return ist3931_write_bus(config, &cmd_buf, true, 1);
}

/**
 * @brief 设置COM引脚映射
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
static uint8_t ist3931_driver_set_com_pad_map(const struct ist3931_config* config) {
  uint8_t cmd_buf[5] = {
    IST3931_CMD_IST_COMMAND_ENTRY, 
    IST3931_CMD_IST_COMMAND_ENTRY,
    IST3931_CMD_IST_COMMAND_ENTRY, 
    IST3931_CMD_IST_COMMAND_ENTRY,
    IST3931_CMD_IST_COM_MAPPING | 1,
    // IST3931_CMD_IST_COM_MAPPING,
  };
  uint8_t cmd_buf2 = IST3931_CMD_EXIT_ENTRY;

  ist3931_write_bus(config, &cmd_buf[0], true, 5);
  config->delay(10);
  ist3931_write_bus(config, &cmd_buf2, true, 1);
  return 0;
}

/**
 * @brief 设置Y地址
 * @param config 配置结构体指针
 * @param y Y坐标
 * @return 0:成功, 1:失败
 */
uint8_t ist3931_driver_set_ay(const struct ist3931_config* config, uint8_t y) {
  uint8_t y_pos = config->y_offset + y;
  uint8_t cmd_buf1 = IST3931_CMD_SET_AY_ADD_LSB | (y_pos & 0x0F);
  uint8_t cmd_buf2 = IST3931_CMD_SET_AY_ADD_MSB | (y_pos >> 4);
  uint8_t cmd_buf[2] = {cmd_buf1, cmd_buf2};

  return ist3931_write_bus(config, cmd_buf, true, 2);
}

/**
 * @brief 设置X地址
 * @param config 配置结构体指针
 * @param x X坐标
 * @return 0:成功, 1:失败
 */
uint8_t ist3931_driver_set_ax(const struct ist3931_config* config, uint8_t x) {
  uint8_t cmd_buf = IST3931_CMD_SET_AX_ADD | (config->x_offset + x);
  return ist3931_write_bus(config, &cmd_buf, true, 1);
}

/**
 * @brief 初始化IST3931控制器
 * @param config 配置结构体指针
 * @return 0:成功, 1:失败
 */
uint8_t ist3931_init(const struct ist3931_config* config) {
  // 设置COM引脚映射
  if (ist3931_driver_set_com_pad_map(config)) {
    return 1;
  }
  config->delay(20);
  
  // 设置占空比
  if (ist3931_set_duty(config)) {
    return 1;
  }
  config->delay(20);
  
  // 设置电源
  if (ist3931_set_power(config)) {
    return 1;
  }
  
  // 设置偏置
  if (ist3931_set_bias(config)) {
    return 1;
  }
  
  // 设置对比度
  if (ist3931_set_ct(config)) {
    return 1;
  }
  
  // 设置帧频
  if (ist3931_set_fr(config)) {
    return 1;
  }
  
  // 设置显示控制
  if (ist3931_driver_display_control(config)) {
    return 1;
  }
  
  // 开启显示
  if (ist3931_driver_set_display_on(config)) {
    return 1;
  }
  
  config->delay(10);
  return 0;
}

/**
 * @brief 按字节方式向屏幕写入数据
 * @param config 配置结构体指针
 * @param x 起始X坐标(字节单位)
 * @param y 起始Y坐标
 * @param width 宽度(字节单位)
 * @param height 高度
 * @param buf 数据缓冲区
 * @return 0:成功, 1:失败
 */
uint8_t ist3931_write_by_byte(const struct ist3931_config* config, const uint8_t x, const uint8_t y,
                              uint8_t width, uint8_t height, const void *buf) {
  uint8_t *data_start = (uint8_t *)buf;
  uint8_t width_tmp = width;
  
  // 检查边界
  if (x + width > IST3931_RAM_WIDTH) {
    width_tmp = IST3931_RAM_WIDTH - x;
  }
  
  if (y + height > IST3931_RAM_HEIGHT) {
    height = IST3931_RAM_HEIGHT - y;
    return 1;
  }
  
  // 设置起始地址
  if (ist3931_driver_set_ay(config, 0)) {
    return 1;
  }
  if (ist3931_driver_set_ax(config, 0)) {
    return 1;
  }

  // 逐行写入数据
  for (uint8_t i = 0; i < height; i++) {
    ist3931_driver_set_ay(config, i + y);
    ist3931_driver_set_ax(config, x);
    ist3931_write_bus(config, data_start, false, width_tmp);
    data_start += width;
  }
  
  return 0;
}

/**
 * @brief 适配不同屏幕的字节写入函数
 * @param config 配置结构体指针
 * @param x 起始X坐标(字节单位)
 * @param y 起始Y坐标
 * @param width 宽度(字节单位)
 * @param height 高度
 * @param buf 数据缓冲区
 * @return 0:成功, 1:失败
 */
uint8_t screen_adapt_write_byte(const struct ist3931_config* config, const uint8_t x, const uint8_t y,
                                uint8_t width, uint8_t height, const void *buf) {
  uint8_t *data_start = (uint8_t *)buf;

  // 逐行写入数据
  for (uint8_t i = 0; i < height; i++) {
    uint8_t ay_true = i + y;
    
    // 适配老王屏幕的特殊地址映射(隔行扫描)
    if (config->type == LAOWANG) {
      ay_true = (ay_true % 2 == 0) ? (ay_true / 2) : ((ay_true - 1) / 2 + 16);
    }
    
    // 设置地址并写入数据
    ist3931_driver_set_ay(config, ay_true);
    ist3931_driver_set_ax(config, x);
    ist3931_write_bus(config, data_start, false, width);
    data_start += width;
  }

  return 0;
}