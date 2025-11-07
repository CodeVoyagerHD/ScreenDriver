/**
 * @file ST7567_LCD.cpp
 * @brief ST7567 LCD显示屏驱动实现
 * @version 1.2
 * @date 2025-11-7
 * @author 航海家-Navigator
 * 
 * @details 基于Adafruit_GFX的ST7567控制器驱动，支持硬件SPI和软件SPI
 * 显示屏规格：128x64像素，单色OLED
 *
 * 主要优化特性：
 * - 显示刷新性能优化（全屏/局部刷新）
 * - 性能统计和帧率测试
 * - 智能刷新策略（根据区域大小自动选择）
 * - 双缓冲支持，消除画面撕裂
 * - 内存使用优化和边界检查
 * - 完整的错误处理和调试支持
 *
 * 引脚连接：
 * - CS:   片选信号
 * - RST:  复位信号
 * - DC:   数据/命令选择
 * - SCLK: 时钟信号（硬件SPI时可省略）
 * - MOSI: 数据输入（硬件SPI时可省略）
 *
 * 使用示例：
 * @code
 * #include "ST7567_LCD.h"
 * 
 * // 引脚定义
 * #define CS_PIN   5
 * #define RST_PIN  4
 * #define DC_PIN   2
 * 
 * ST7567_LCD lcd(CS_PIN, RST_PIN, DC_PIN);
 * 
 * void setup() {
 *     lcd.begin(0x20);
 *     lcd.setTextSize(1);
 *     lcd.setTextColor(WHITE);
 *     lcd.setCursor(10, 10);
 *     lcd.print("Hello ST7567!");
 *     lcd.display();
 * }
 * @endcode
 */

#ifndef __ST7567_LCD_H
#define __ST7567_LCD_H

#include <Adafruit_GFX.h> // Adafruit图形库基类
#include <SPI.h>          // ESP32 SPI库
#include <Arduino.h>      // Arduino基础库

class ST7567_LCD : public Adafruit_GFX
{
public:
    // 常量定义
    static const uint16_t LCD_WIDTH = 128;     ///< 显示屏宽度（像素）
    static const uint16_t LCD_HEIGHT = 64;     ///< 显示屏高度（像素）
    static const uint16_t FRAME_SIZE = (LCD_WIDTH * LCD_HEIGHT / 8); ///< 帧缓冲区大小（字节）

    /**
     * @brief 硬件SPI构造函数
     * @param cs   片选引脚
     * @param rst  复位引脚
     * @param dc   数据/命令选择引脚
     * @param spi  SPI实例（默认使用SPI，也可使用VSPI或HSPI）
     * @param frequency SPI时钟频率（默认40MHz）
     */
    ST7567_LCD(int8_t cs, int8_t rst, int8_t dc, SPIClass &spi = SPI, uint32_t frequency = 40000000);

    /**
     * @brief 软件SPI构造函数（向后兼容）
     * @param cs   片选引脚
     * @param rst  复位引脚
     * @param dc   数据/命令选择引脚
     * @param sclk 时钟引脚
     * @param mosi 数据输入引脚
     */
    ST7567_LCD(int8_t cs, int8_t rst, int8_t dc, int8_t sclk, int8_t mosi);

    /**
     * @brief 析构函数 - 自动释放帧缓冲区内存
     */
    ~ST7567_LCD();

    // 基本显示控制函数
    /**
     * @brief 初始化显示屏
     * @param contrast 对比度值（0x00-0xFF，默认0x20）
     * 
     * 初始化流程：
     * 1. 配置引脚模式和SPI接口
     * 2. 执行硬件复位序列
     * 3. 发送初始化命令序列
     * 4. 设置对比度和清空显示
     */
    void begin(uint8_t contrast = 0x20);

    /**
     * @brief 将帧缓冲区内容刷新到显示屏（全屏刷新）
     * 
     * 优化特性：
     * - 批量传输减少SPI事务开销
     * - 显示状态检查，节能优化
     * - 性能统计和FPS计算
     */
    void display();

    /**
     * @brief 智能局部刷新函数
     * @param x 起始X坐标（0-127）
     * @param y 起始Y坐标（0-63）
     * @param width 刷新区域宽度
     * @param height 刷新区域高度
     * 
     * 智能特性：
     * - 自动边界检查和区域裁剪
     * - 根据区域大小自动选择刷新策略
     * - 跨页区域的连续刷新优化
     */
    void refreshRegion(int16_t x, int16_t y, uint16_t width, uint16_t height);

    /**
     * @brief 清空帧缓冲区（不立即显示）
     * 
     * @note 此操作只清空缓冲区，需要调用display()才能实际更新显示
     */
    void clearDisplay();

    /**
     * @brief 快速清屏并立即刷新
     * @param pattern 填充模式（0x00:全黑, 0xFF:全白, 其他:图案）
     * 
     * 优化特性：
     * - 直接操作显示内存，跳过帧缓冲区
     * - 使用图案填充减少数据传输
     * - 支持快速清屏模式
     */
    void clearScreen(uint8_t pattern = 0x00);

    /**
     * @brief 设置显示反色
     * @param invert true:反色显示, false:正常显示
     */
    void invertDisplay(bool invert);

    /**
     * @brief 设置对比度
     * @param contrast 对比度值（0x00-0xFF）
     * 
     * 建议范围：0x10-0x30
     */
    void setContrast(uint8_t contrast);

    // 高级显示控制函数
    /**
     * @brief 设置显示使能状态
     * @param enable true:开启显示, false:关闭显示
     * 
     * 应用场景：
     * - 节能模式：关闭显示降低功耗
     * - 闪烁效果：快速开关创建闪烁
     * - 数据传输期间避免显示杂讯
     */
    void setDisplayEnabled(bool enable);

    /**
     * @brief 双缓冲切换显示
     * @param newBuffer 新帧缓冲区指针（为nullptr时自动创建）
     * 
     * 双缓冲优势：
     * - 消除画面撕裂
     * - 提高渲染流畅度
     * - 支持后台渲染
     */
    void swapBuffers(uint8_t* newBuffer = nullptr);

    /**
     * @brief 设置睡眠模式
     * @param sleep true:进入睡眠, false:退出睡眠
     * 
     * 睡眠模式特性：
     * - 显著降低功耗
     * - 保持显示内容
     * - 快速唤醒恢复
     */
    void setSleepMode(bool sleep);

    /**
     * @brief 设置显示起始行
     * @param line 起始行号（0-63）
     * 
     * 应用场景：
     * - 实现垂直滚动效果
     * - 显示偏移校正
     * - 动画特效
     */
    void setStartLine(uint8_t line);

    /**
     * @brief 设置页地址（用于高级操作）
     * @param page 页地址（0-7）
     */
    void setPageAddress(uint8_t page);

    /**
     * @brief 设置列地址（用于高级操作）
     * @param col 列地址（0-131）
     */
    void setColumnAddress(uint8_t col);

    // 图形绘制函数（重写Adafruit_GFX）
    /**
     * @brief 绘制像素点（重写Adafruit_GFX虚函数）
     * @param x 像素点X坐标
     * @param y 像素点Y坐标
     * @param color 颜色（0:清除, 1:设置）
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

    /**
     * @brief 绘制水平线（优化版本）
     * @param x 起始X坐标
     * @param y 起始Y坐标
     * @param w 线宽度
     * @param color 颜色
     */
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;

    /**
     * @brief 绘制垂直线（优化版本）
     * @param x 起始X坐标
     * @param y 起始Y坐标
     * @param h 线高度
     * @param color 颜色
     */
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) override;

    /**
     * @brief 填充矩形（优化版本）
     * @param x 矩形左上角X坐标
     * @param y 矩形左上角Y坐标
     * @param w 矩形宽度
     * @param h 矩形高度
     * @param color 填充颜色
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

    // 数据操作函数
    /**
     * @brief 批量写入缓冲区数据到显示屏
     * @param buffer 数据缓冲区指针
     * @param length 数据长度
     * 
     * @note 此方法绕过帧缓冲区，直接写入显示RAM
     * 适用于需要高速直接写入的场景
     */
    void writeBuffer(const uint8_t *buffer, size_t length);

    /**
     * @brief 获取帧缓冲区指针
     * @return 帧缓冲区指针
     */
    uint8_t* getFrameBuffer() { return frameBuffer; }

    /**
     * @brief 获取帧缓冲区大小
     * @return 帧缓冲区大小（字节）
     */
    size_t getFrameBufferSize() { return FRAME_SIZE; }

    // 调试和测试函数
    /**
     * @brief 读取显示状态（扩展功能）
     * @return 显示状态字节
     * 
     * @note 此功能需要硬件支持MISO引脚
     */
    uint8_t readStatus();

    /**
     * @brief 性能测试函数（调试用）
     * @param iterations 测试迭代次数
     * @return 平均刷新时间（微秒）
     */
    uint32_t performanceTest(uint16_t iterations = 1);

    /**
     * @brief 显示测试图案（调试用）
     * @param pattern 测试图案类型
     * 
     * 测试图案类型：
     * - 0: 棋盘格图案
     * - 1: 绘制移动矩形
     * - 2: 边框图案
     * - 3: 填充测试
     */
    void testPattern(uint8_t pattern = 0);

    /**
     * @brief 获取当前帧率统计
     * @return 当前帧率（FPS）
     */
    uint16_t getFPS();

private:
    // 私有方法
    /**
     * @brief 写入命令字节
     * @param cmd 命令字节
     */
    void writeCommand(uint8_t cmd);

    /**
     * @brief 写入单字节数据
     * @param data 数据字节
     */
    void writeData(uint8_t data);

    /**
     * @brief 批量写入数据
     * @param data 数据缓冲区指针
     * @param len 数据长度
     */
    void writeDataBulk(const uint8_t *data, size_t len);

    /**
     * @brief 设置显示地址窗口
     * @param page 页地址（0-7）
     * @param col 列地址（0-131）
     */
    void setAddrWindow(uint8_t page, uint8_t col);

    /**
     * @brief 初始化显示屏控制器
     */
    void initDisplay();

    /**
     * @brief 开始SPI事务
     */
    void spiBeginTransaction();

    /**
     * @brief 结束SPI事务
     */
    void spiEndTransaction();

    // 引脚定义
    int8_t _cs;   ///< 片选引脚
    int8_t _rst;  ///< 复位引脚
    int8_t _dc;   ///< 数据/命令选择引脚
    int8_t _sclk; ///< 时钟引脚（软件SPI时使用）
    int8_t _mosi; ///< 数据输入引脚（软件SPI时使用）

    // 显示控制参数
    uint8_t _contrast;           ///< 当前对比度值
    bool _useHardwareSPI;        ///< 使用硬件SPI标志
    uint8_t *frameBuffer;        ///< 帧缓冲区指针（128x64/8 = 1024字节）
    
    // 显示状态控制
    bool _displayEnabled;         ///< 显示使能标志

    // 性能统计
    uint32_t _lastStatTime;       ///< 上次统计时间
    uint16_t _frameCount;         ///< 帧计数器
    uint16_t _fps;                ///< 当前帧率

    // 硬件SPI相关成员
    SPIClass *_spi;               ///< SPI实例指针
    SPISettings _spiSettings;     ///< SPI配置参数
    uint32_t _spiFrequency;       ///< SPI时钟频率
};

#endif // __ST7567_LCD_H