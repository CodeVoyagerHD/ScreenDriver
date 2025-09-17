# IST3931 LCD屏幕驱动库使用指南

[toc]

## 零、老王IST3931 LCD屏幕（0.4寸-32x64-14pin-IIC）

引脚定义:
- 1-6	 NC

- 7	  GND

- 8	  VCC(3.3V)

- 9	  RST(10K上拉电阻或接入单片机)

- 10	SCL(10K上拉电阻)

- 11	SDA(10K上拉电阻)

- 12	NC
- 13	VG(1uF电容到GND)

- 14	NC

实物图:

|  正面    |  背面    |
| ---- | ---- |
|  <img src="./img/1 (1).jpg" alt="这是图片" style="zoom: 25%;" />    |  <img src="./img/1 (2).jpg" alt="这是图片" style="zoom: 28%;" />  |



## 一、驱动库概述
本驱动库专为IST3931控制器（老王屏幕）设计，使用VSCode+Platformio+Arduino平台编译ESP8266开发板实现。本驱动库提供完整的字符显示功能，支持多种字体和显示模式。

核心功能包括：

- 屏幕初始化与配置
- 多尺寸字体渲染（6x8/8x16/12x24）
- 字符/字符串显示
- 多种显示模式（正常/反色/异或/覆盖）
- 屏幕缓冲区管理

引脚连接：

- RST	3.3v（直接连接到ESP8266的RST引脚）
- SCL	D5(5)
- SDA	D4(4)

文件结构：

```cpp
main.cpp					//主程序  
display_char.cpp/.h			//字符串显示函数及声明
display_font.cpp/.h			//字符取模文件
display_for_laowang.cpp/.h  //IST3931屏幕初始化配置函数（必须有）
display_ist3931.cpp/.h		//ist3931屏幕初始化相关函数（必须有）
```

## 二、硬件配置要求
```cpp
define HEIGHT_PIX 32   // 屏幕高度(像素)

define WIDTH_PIX 64    // 屏幕宽度(像素)

define IST3931_ADDR 0x3F // I2C设备地址
```

## 三、快速入门

### 0.自定义配置

可自定义修改`display_for_laowang.cpp`文件内**IIC写入函数**``uint8_t zxc_i2c_write_only(uint8_t device_addr, uint8_t* data, uint16_t len) `和**延时函数**`void zxc_delay_ms(uint16_t ms) `适配不同平台

```cpp
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

```
### 1. 初始化屏幕
```cpp
void setup() {
    // 初始化I2C和屏幕
    Wire.begin();
    display_for_laowang_init();
    clear_screen(0); // 清屏为黑色
}
```

### 2. 显示字符
```cpp
display_char(10, 5, 'A', FONT_SIZE_8x16, MODE_NORMAL);
```

### 3. 显示字符串
```cpp
display_string(0, 0, "Hello World!", 
               FONT_SIZE_8x16, MODE_NORMAL, 1);
```

## 四、核心功能详解
### 1. 字体系统
支持三种字体尺寸：
```c
typedef enum {
    FONT_SIZE_6x8,    // 小字体
    FONT_SIZE_8x16,   // 标准字体
    FONT_SIZE_12x24   // 大字体
} font_size_t;
```

字体数据结构：
```cpp
typedef struct {
    uint8_t width;          // 字符宽度(像素)
    uint8_t height;         // 字符高度(像素)
    uint8_t bytes_per_char; // 字符数据字节数
    const uint8_t *data;    // 字模数据指针
} font_t;
```

### 2. 显示模式
```cpp
typedef enum {
    MODE_NORMAL,      // 正常显示
    MODE_INVERT,      // 反色显示
    MODE_OVERWRITE,   // 覆盖背景
    MODE_XOR          // 异或模式（用于闪烁效果）
} char_display_mode_t;
```

### 3. 屏幕写入函数
核心像素写入函数：
```cpp
uint8_t screen_write_by_pix(uint8_t x, uint8_t y, 
                            uint8_t width, uint8_t height, 
                            const void *buf);

参数说明：
- `x`, `y`: 起始坐标（像素）
- `width`, `height`: 写入区域尺寸
- `buf`: 像素数据缓冲区（按位存储）
```
### 4. 字符显示实现
字符渲染流程：
1. 获取字体数据：`get_font(font_size_t size)`
2. 检查ASCII范围(32-126)
3. 计算字模偏移量：`(c - 32) * font_ptr->bytes_per_char`
4. 处理显示模式（特殊模式需创建临时缓冲区）
5. 调用`screen_write_by_pix`写入屏幕

## 五、高级用法
### 1. 自定义显示模式
```cpp
// 反色显示示例
display_char(20, 10, 'B', FONT_SIZE_12x24, MODE_INVERT);

// 异或模式（可做闪烁效果）
display_string(5, 20, "XOR MODE", 
               FONT_SIZE_6x8, MODE_XOR, 0);
```

### 2. 屏幕缓冲管理
使用全局缓冲区：
```cpp
static uint8_t screen_buf[HEIGHT_PIX][WIDTH_PIX / 8];

直接操作缓冲区可实现高效局部刷新
```
## 六、配置参数详解
IST3931初始化配置模板：
```cpp
static const struct ist3931_config config = {
    .type = LAOWANG,
    .vc = 1,        // 电压转换电路使能
    .vf = 1,        // 电压跟随电路使能
    .bias = 2,      // 偏置比(0-7)
    .ct = 150,      // 对比度(0-255)
    .duty = 32,     // 扫描占空比(1-64)
    .fr = 60,       // 帧频分频比
    // 更多参数...
};
```

## 七、常见问题
1. **字符显示不全**
   - 检查坐标是否越界：`x + width > WIDTH_PIX`
   - 确保字符在`ASCII 32-126`范围内

2. **显示错位**
  
   - 老王屏幕采用特殊地址映射：
     ```cpp
     ay_true = (y % 2 == 0) ? (y/2) : ((y-1)/2 + 16);
     ```

3. **I2C通信失败**
   - 检查设备地址`0x3F`
   - 确认`zxc_i2c_write_only()`实现正确

## 八、性能优化建议
1. 优先使用`MODE_OVERWRITE`（直接写入模式）
2. 避免频繁局部刷新，使用双缓冲机制
3. 对于静态文本，可缓存渲染结果

## 九、示例工程
```cpp
include "display_char.h"

void setup() {
    display_for_laowang_init();
    clear_screen(0);
    
    // 显示多行不同字体
    display_string(5, 0, "6x8 Font", FONT_SIZE_6x8, MODE_NORMAL, 1);
    display_string(5, 10, "8x16 Font", FONT_SIZE_8x16, MODE_OVERWRITE, 1);
    display_string(5, 30, "12x24", FONT_SIZE_12x24, MODE_INVERT, 0);
}

void loop() {
    // 异或模式实现闪烁效果
    static bool toggle = false;
    display_string(20, 20, "BLINK", 
                   FONT_SIZE_8x16, 
                   toggle ? MODE_XOR : MODE_NORMAL, 
                   1);
    toggle = !toggle;
    delay(500);
}
```

## 十、资源链接地址

- [IST3919屏幕资料手册](./img/IST3931-20200529.pdf)
- [github项目链接:https://github.com/CodeVoyagerHD/ScreenDriver](https://github.com/CodeVoyagerHD/ScreenDriver)

## 十一、参考资料

- [https://github.com/ZxcSpace/zxc_others](https://github.com/ZxcSpace/zxc_others)
- [https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/display/display_ist3931.c](https://github.com/zephyrproject-rtos/zephyr/blob/main/drivers/display/display_ist3931.c)


> **提示**：完整代码库包含在display_char.cpp、display_font.cpp等文件中，使用时需将所有文件加入项目