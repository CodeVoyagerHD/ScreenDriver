/**
 * @file ST7567_LCD.cpp
 * @brief ST7567 LCD显示屏驱动实现
 * @version 1.1
 * @date 2024-01-01
 */

#include "ST7567_LCD.h"

// 显示屏物理参数定义
#define LCD_WIDTH 128                           ///< 显示屏宽度（像素）
#define LCD_HEIGHT 64                           ///< 显示屏高度（像素）
#define FRAME_SIZE (LCD_WIDTH * LCD_HEIGHT / 8) ///< 帧缓冲区大小（字节）

/**
 * @brief 初始化命令序列
 *
 * 命令说明：
 * 0xE2 - 系统复位
 * 0xAE - 关闭显示
 * 0x40 - 设置显示起始行为0
 * 0xA0 - 段重映射正常（列0映射到SEG0）
 * 0xC8 - 输出扫描方向反向（COM0到COM63）
 * 0xA6 - 正常显示（非反色）
 * 0xA2 - 偏置比1/9
 * 0x2F - 内部电源控制（升压器、稳压器全开）
 * 0xF8,0x00 - 设置偏置比1/9
 * 0x24 - 电阻比调节
 * 0x81,0x10 - 设置对比度（后续会重新设置）
 * 0xAC,0x00 - 关闭静态指示器
 * 0xAF - 开启显示
 */
static const uint8_t init_cmds[] = {
    0xE2,       // 系统复位
    0xAE,       // 关闭显示
    0x40,       // 设置显示起始行
    0xA0,       // 段重映射正常（0xA0:正常, 0xA1:反向）
    0xC8,       // 输出扫描方向（0xC0:正常, 0xC8:反向）
    0xA6,       // 正常显示（0xA6:正常, 0xA7:反显）
    0xA2,       // 偏置比1/9
    0x2F,       // 内部电源控制
    0xF8, 0x00, // 设置偏置比1/9
    0x24,       // 电阻比调节
    0x81, 0x10, // 设置对比度
    0xAC, 0x00, // 关闭静态指示器
    0xAF        // 开启显示
};

/**
 * @brief 硬件SPI构造函数
 * @param cs 片选引脚
 * @param rst 复位引脚
 * @param dc 数据/命令选择引脚
 * @param spi SPI实例引用
 * @param frequency SPI时钟频率
 */
ST7567_LCD::ST7567_LCD(int8_t cs, int8_t rst, int8_t dc, SPIClass &spi, uint32_t frequency)
    : Adafruit_GFX(LCD_WIDTH, LCD_HEIGHT), _spi(&spi), _spiFrequency(frequency)
{
    _cs = cs;
    _rst = rst;
    _dc = dc;
    _sclk = _mosi = -1; // 硬件SPI时时钟和数据引脚为-1
    _useHardwareSPI = true;

    // 配置SPI参数：频率、位序、模式
    _spiSettings = SPISettings(_spiFrequency, MSBFIRST, SPI_MODE0);

    // 分配帧缓冲区内存并初始化为0
    frameBuffer = new uint8_t[FRAME_SIZE]();
}

/**
 * @brief 软件SPI构造函数（向后兼容）
 * @param cs 片选引脚
 * @param rst 复位引脚
 * @param dc 数据/命令选择引脚
 * @param sclk 时钟引脚
 * @param mosi 数据输入引脚
 */
ST7567_LCD::ST7567_LCD(int8_t cs, int8_t rst, int8_t dc, int8_t sclk, int8_t mosi)
    : Adafruit_GFX(LCD_WIDTH, LCD_HEIGHT), _spi(nullptr)
{
    _cs = cs;
    _rst = rst;
    _dc = dc;
    _sclk = sclk;
    _mosi = mosi;
    // 如果sclk或mosi为-1，则使用硬件SPI
    _useHardwareSPI = (sclk == -1 || mosi == -1);
    frameBuffer = new uint8_t[FRAME_SIZE]();
}

/**
 * @brief 初始化显示屏
 * @param contrast 初始对比度值
 *
 * 初始化流程：
 * 1. 配置引脚模式
 * 2. 初始化SPI接口
 * 3. 执行硬件复位序列
 * 4. 发送初始化命令序列
 * 5. 设置对比度
 * 6. 清空显示缓冲区
 */
void ST7567_LCD::begin(uint8_t contrast)
{
    // 配置控制引脚为输出模式
    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_rst, OUTPUT);

    // 根据模式初始化SPI
    if (_useHardwareSPI && _spi)
    {
        _spi->begin();
        Serial.println("ST7567: Hardware SPI initialized");
    }
    else
    {
        pinMode(_sclk, OUTPUT);
        pinMode(_mosi, OUTPUT);
        Serial.println("ST7567: Software SPI initialized");
    }

    // 硬件复位序列
    digitalWrite(_rst, HIGH); // 确保复位引脚为高
    delay(10);
    digitalWrite(_rst, LOW); // 拉低复位引脚
    delay(10);
    digitalWrite(_rst, HIGH); // 释放复位引脚
    delay(10);

    // 初始化片选信号
    digitalWrite(_cs, HIGH);

    // 发送初始化命令并设置参数
    initDisplay();
    setContrast(contrast);
    clearDisplay();
    display(); // 首次显示清空屏幕
}

/**
 * @brief 开始SPI事务
 *
 * 在SPI传输前调用，设置SPI参数并获取总线控制权
 * 确保传输过程不被其他设备中断
 */
void ST7567_LCD::spiBeginTransaction()
{
    if (_useHardwareSPI && _spi)
    {
        _spi->beginTransaction(_spiSettings);
    }
}

/**
 * @brief 结束SPI事务
 *
 * 在SPI传输完成后调用，释放SPI总线控制权
 */
void ST7567_LCD::spiEndTransaction()
{
    if (_useHardwareSPI && _spi)
    {
        _spi->endTransaction();
    }
}

/**
 * @brief 初始化显示屏控制器
 *
 * 发送初始化命令序列到ST7567控制器
 * 配置显示参数和控制器工作模式
 */
void ST7567_LCD::initDisplay()
{
    spiBeginTransaction();

    // 逐条发送初始化命令
    for (size_t i = 0; i < sizeof(init_cmds); i++)
    {
        writeCommand(init_cmds[i]);
    }

    spiEndTransaction();
}

/**
 * @brief 写入命令字节
 * @param cmd 命令字节
 *
 * 传输流程：
 * 1. 设置DC为低电平（命令模式）
 * 2. 拉低CS片选信号
 * 3. 通过SPI发送命令字节
 * 4. 拉高CS片选信号
 */
void ST7567_LCD::writeCommand(uint8_t cmd)
{
    digitalWrite(_dc, LOW); // 命令模式
    digitalWrite(_cs, LOW); // 使能器件

    if (_useHardwareSPI && _spi)
    {
        _spi->transfer(cmd); // 硬件SPI传输
    }
    else
    {
        shiftOut(_mosi, _sclk, MSBFIRST, cmd); // 软件SPI传输
    }

    digitalWrite(_cs, HIGH); // 禁用器件
}

/**
 * @brief 写入单字节数据
 * @param data 数据字节
 *
 * 传输流程与writeCommand类似，但DC引脚为高电平（数据模式）
 */
void ST7567_LCD::writeData(uint8_t data)
{
    digitalWrite(_dc, HIGH); // 数据模式
    digitalWrite(_cs, LOW);  // 使能器件

    if (_useHardwareSPI && _spi)
    {
        _spi->transfer(data); // 硬件SPI传输
    }
    else
    {
        shiftOut(_mosi, _sclk, MSBFIRST, data); // 软件SPI传输
    }

    digitalWrite(_cs, HIGH); // 禁用器件
}

/**
 * @brief 批量写入数据
 * @param data 数据缓冲区指针
 * @param len 数据长度
 *
 * 优化传输效率的方法：
 * - 减少CS引脚切换次数
 * - 硬件SPI下可连续传输多个字节
 * - 适合大量数据的连续写入
 */
void ST7567_LCD::writeDataBulk(const uint8_t *data, size_t len)
{
    if (len == 0)
        return;

    digitalWrite(_dc, HIGH); // 数据模式
    digitalWrite(_cs, LOW);  // 使能器件

    if (_useHardwareSPI && _spi)
    {
        // 硬件SPI批量传输（效率更高）
        for (size_t i = 0; i < len; i++)
        {
            _spi->transfer(data[i]);
        }
    }
    else
    {
        // 软件SPI逐个传输
        for (size_t i = 0; i < len; i++)
        {
            shiftOut(_mosi, _sclk, MSBFIRST, data[i]);
        }
    }

    digitalWrite(_cs, HIGH); // 禁用器件
}

/**
 * @brief 设置显示地址窗口
 * @param page 页地址（0-7）
 * @param col 列地址（0-131）
 *
 * ST7567显示内存组织：
 * - 8页（page），每页8行像素
 * - 132列（col），实际显示128列
 * - 地址命令：页地址 + 列地址高4位 + 列地址低4位
 */
void ST7567_LCD::setAddrWindow(uint8_t page, uint8_t col)
{
    writeCommand(0xB0 + page);        // 设置页地址（0xB0-0xB7）
    writeCommand(0x10 + (col >> 4));  // 设置列地址高4位（0x10-0x1F）
    writeCommand(0x00 + (col & 0xF)); // 设置列地址低4位（0x00-0x0F）
}

/**
 * @brief 设置局部显示窗口并刷新
 * @param page 起始页
 * @param col 起始列
 * @param width 窗口宽度
 * @param height 窗口高度（页数）
 *
 * 适用于局部刷新场景，减少数据传输量：
 * - 游戏中的精灵动画
 * - 文本编辑器的光标闪烁
 * - 进度条更新
 */
void ST7567_LCD::setPartialWindow(uint8_t page, uint8_t col, uint8_t width, uint8_t height)
{
    // 计算结束地址（限制在有效范围内）
    uint8_t endPage = min((uint8_t)7, (uint8_t)(page + height - 1));
    uint8_t endCol = min((uint8_t)131, (uint8_t)(col + width - 1));

    spiBeginTransaction();

    // 逐页刷新指定区域
    for (uint8_t p = page; p <= endPage; p++)
    {
        setAddrWindow(p, col);
        // 批量传输该页的指定列数据
        writeDataBulk(&frameBuffer[p * LCD_WIDTH + col], endCol - col + 1);
    }

    spiEndTransaction();
}

/**
 * @brief 绘制像素点
 * @param x X坐标（0-127）
 * @param y Y坐标（0-63）
 * @param color 颜色（0:清除, 非0:设置）
 *
 * 像素存储格式：
 * - 垂直8像素为一页，MSB在顶部
 * - 帧缓冲区索引：page * 128 + column
 * - 位位置：y % 8
 */
void ST7567_LCD::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    // 边界检查
    if (x < 0 || x >= width() || y < 0 || y >= height())
        return;

    // 计算帧缓冲区中的位置
    uint16_t idx = (y / 8) * LCD_WIDTH + x; // 字节索引
    uint8_t bit = 1 << (y % 8);             // 位掩码

    // 设置或清除指定位
    if (color)
    {
        frameBuffer[idx] |= bit; // 设置像素
    }
    else
    {
        frameBuffer[idx] &= ~bit; // 清除像素
    }
}

/**
 * @brief 刷新显示（将帧缓冲区内容发送到显示屏）
 *
 * 刷新流程：
 * 1. 遍历所有8页
 * 2. 设置每页的起始地址
 * 3. 批量传输该页的128字节数据
 * 4. 使用SPI事务确保传输完整性
 */
void ST7567_LCD::display()
{
    spiBeginTransaction();

    // 逐页刷新整个屏幕
    for (uint8_t page = 0; page < 8; page++)
    {
        setAddrWindow(page, 0);                                   // 设置页地址和列起始地址
        writeDataBulk(&frameBuffer[page * LCD_WIDTH], LCD_WIDTH); // 传输整页数据
    }

    spiEndTransaction();
}

/**
 * @brief 直接写入缓冲区数据到显示屏
 * @param buffer 数据缓冲区
 * @param length 数据长度
 *
 * 适用于：
 * - 预先渲染好的图像数据
 * - 从外部存储器加载的图形
 * - 需要绕过帧缓冲区的特殊应用
 */
void ST7567_LCD::writeBuffer(const uint8_t *buffer, size_t length)
{
    if (length > FRAME_SIZE)
        length = FRAME_SIZE;

    spiBeginTransaction();
    setAddrWindow(0, 0);           // 设置到显示起始位置
    writeDataBulk(buffer, length); // 直接写入数据
    spiEndTransaction();
}

/**
 * @brief 清空帧缓冲区
 *
 * 将帧缓冲区所有字节设置为0，即清空屏幕
 * 注意：此操作只清空缓冲区，需要调用display()才能实际更新显示
 */
void ST7567_LCD::clearDisplay()
{
    memset(frameBuffer, 0x00, FRAME_SIZE);
}

/**
 * @brief 设置显示反色
 * @param invert 反色标志
 *
 * 反色显示效果：
 * - 正常显示：亮像素在暗背景上
 * - 反色显示：暗像素在亮背景上
 */
void ST7567_LCD::invertDisplay(bool invert)
{
    spiBeginTransaction();
    writeCommand(invert ? 0xA7 : 0xA6); // 0xA7:反色, 0xA6:正常
    spiEndTransaction();
}

/**
 * @brief 设置对比度
 * @param contrast 对比度值（0x00-0xFF）
 *
 * 对比度调节：
 * - 较低值：显示较淡
 * - 较高值：显示较深
 * - 建议范围：0x10-0x30
 */
void ST7567_LCD::setContrast(uint8_t contrast)
{
    _contrast = contrast;
    spiBeginTransaction();
    writeCommand(0x81);      // 对比度设置命令
    writeCommand(_contrast); // 对比度值
    spiEndTransaction();
}

/**
 * @brief 使用示例
 *
 * // 引脚定义
 * #define CS_PIN   5
 * #define RST_PIN  4
 * #define DC_PIN   2
 *
 * // 使用硬件SPI（推荐）
 * ST7567_LCD lcd(CS_PIN, RST_PIN, DC_PIN);
 *
 * void setup() {
 *     lcd.begin(0x20); // 初始化并设置对比度
 *     lcd.setTextSize(1);
 *     lcd.setTextColor(WHITE);
 *
 *     // 显示文本
 *     lcd.setCursor(10, 10);
 *     lcd.print("Hello ST7567!");
 *     lcd.display(); // 刷新到屏幕
 *
 *     // 局部刷新示例
 *     lcd.setPartialWindow(0, 0, 64, 32); // 刷新左上角区域
 * }
 *
 * void loop() { }
 */