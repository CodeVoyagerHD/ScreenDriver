/**
 * @file ST7567_LCD.h
 * @brief ST7567 LCD显示屏驱动库 for ESP32
 * @version 1.1
 * @date 2024-01-01
 *
 * @details 基于Adafruit_GFX的ST7567控制器驱动，支持硬件SPI和软件SPI
 * 显示屏规格：128x64像素，单色OLED
 *
 * 引脚连接：
 * - CS:   片选信号
 * - RST:  复位信号
 * - DC:   数据/命令选择
 * - SCLK: 时钟信号（硬件SPI时可省略）
 * - MOSI: 数据输入（硬件SPI时可省略）
 */

#ifndef __ST7567_LCD_H
#define __ST7567_LCD_H

#include <Adafruit_GFX.h> // Adafruit图形库基类
#include <SPI.h>          // ESP32 SPI库

class ST7567_LCD : public Adafruit_GFX
{
public:
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
     * @brief 初始化显示屏
     * @param contrast 对比度值（0x00-0xFF，默认0x20）
     */
    void begin(uint8_t contrast = 0x20);

    /**
     * @brief 将帧缓冲区内容刷新到显示屏
     */
    void display();

    /**
     * @brief 清空帧缓冲区（不立即显示）
     */
    void clearDisplay();

    /**
     * @brief 设置显示反色
     * @param invert true:反色显示, false:正常显示
     */
    void invertDisplay(bool invert);

    /**
     * @brief 设置对比度
     * @param contrast 对比度值（0x00-0xFF）
     */
    void setContrast(uint8_t contrast);

    /**
     * @brief 绘制像素点（重写Adafruit_GFX虚函数）
     * @param x 像素点X坐标
     * @param y 像素点Y坐标
     * @param color 颜色（0:清除, 1:设置）
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;

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
     * @brief 设置局部显示窗口并刷新
     * @param page 起始页地址（0-7）
     * @param col  起始列地址（0-131）
     * @param width 窗口宽度
     * @param height 窗口高度（以页为单位）
     *
     * @note 用于局部刷新，提高刷新效率
     * 只更新指定区域，减少数据传输量
     */
    void setPartialWindow(uint8_t page, uint8_t col, uint8_t width, uint8_t height);

private:
    // 私有方法
    void writeCommand(uint8_t cmd);                      ///< 写入命令字节
    void writeData(uint8_t data);                        ///< 写入单字节数据
    void writeDataBulk(const uint8_t *data, size_t len); ///< 批量写入数据
    void setAddrWindow(uint8_t page, uint8_t col);       ///< 设置显示地址窗口
    void initDisplay();                                  ///< 初始化显示屏控制器
    void spiBeginTransaction();                          ///< 开始SPI事务
    void spiEndTransaction();                            ///< 结束SPI事务

    // 引脚定义
    int8_t _cs;   ///< 片选引脚
    int8_t _rst;  ///< 复位引脚
    int8_t _dc;   ///< 数据/命令选择引脚
    int8_t _sclk; ///< 时钟引脚（软件SPI时使用）
    int8_t _mosi; ///< 数据输入引脚（软件SPI时使用）

    uint8_t _contrast;    ///< 当前对比度值
    bool _useHardwareSPI; ///< 使用硬件SPI标志
    uint8_t *frameBuffer; ///< 帧缓冲区指针（128x64/8 = 1024字节）

    // 硬件SPI相关成员
    SPIClass *_spi;           ///< SPI实例指针
    SPISettings _spiSettings; ///< SPI配置参数
    uint32_t _spiFrequency;   ///< SPI时钟频率
};

#endif // __ST7567_ESP32_H