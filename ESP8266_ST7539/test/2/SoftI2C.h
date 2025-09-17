#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <Arduino.h>

class SoftI2C {
public:
    // 初始化I2C引脚（构造函数）
    SoftI2C(uint8_t scl_pin, uint8_t sda_pin);

    // I2C基本操作
    uint8_t writeData(uint8_t slaveAddress, uint8_t* data, uint16_t len);
    uint8_t readData(uint8_t slaveAddress, uint8_t* data, uint8_t len);

private:
    // GPIO引脚定义
    uint8_t _scl_pin;
    uint8_t _sda_pin;

    // 底层操作函数
    void start(void);
    void stop(void);
    void ack(void);
    void nack(void);
    uint8_t waitAck(void);
    void sendByte(uint8_t sendByte);
    uint8_t readByte(void);
    void delay(void);
    void sdaIn(void);
    void sdaOut(void);
};

#endif