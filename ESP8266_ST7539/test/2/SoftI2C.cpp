#include "SoftI2C.h"

// 构造函数初始化引脚
SoftI2C::SoftI2C(uint8_t scl_pin, uint8_t sda_pin) 
    : _scl_pin(scl_pin), _sda_pin(sda_pin) {
    pinMode(_scl_pin, OUTPUT_OPEN_DRAIN);
    pinMode(_sda_pin, OUTPUT_OPEN_DRAIN);
    digitalWrite(_scl_pin, HIGH);
    digitalWrite(_sda_pin, HIGH);
}

// 微秒级延时（可根据I2C速度需求调整）
void SoftI2C::delay(void) {
    delayMicroseconds(5);  // 100kHz I2C时钟对应值
}

// SDA方向控制
void SoftI2C::sdaIn(void) {
    pinMode(_sda_pin, INPUT_PULLUP);
}

void SoftI2C::sdaOut(void) {
    pinMode(_sda_pin, OUTPUT_OPEN_DRAIN);
    digitalWrite(_sda_pin, HIGH);  // 默认释放总线
}

// 基础信号函数
void SoftI2C::start(void) {
    sdaOut();
    digitalWrite(_sda_pin, HIGH);
    digitalWrite(_scl_pin, HIGH);
    delay();
    digitalWrite(_sda_pin, LOW);
    delay();
    digitalWrite(_scl_pin, LOW);
    delay();
}

void SoftI2C::stop(void) {
    sdaOut();
    digitalWrite(_scl_pin, LOW);
    digitalWrite(_sda_pin, LOW);
    delay();
    digitalWrite(_scl_pin, HIGH);
    digitalWrite(_sda_pin, HIGH);
    delay();
}

// 数据读写函数
void SoftI2C::sendByte(uint8_t sendByte) {
    sdaOut();
    digitalWrite(_scl_pin, LOW);
    
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_sda_pin, (sendByte & 0x80) ? HIGH : LOW);
        sendByte <<= 1;
        delay();
        digitalWrite(_scl_pin, HIGH);
        delay();
        digitalWrite(_scl_pin, LOW);
        delay();
    }
}

uint8_t SoftI2C::readByte(void) {
    uint8_t receiveByte = 0;
    sdaIn();
    
    for (uint8_t i = 0; i < 8; i++) {
        digitalWrite(_scl_pin, LOW);
        delay();
        digitalWrite(_scl_pin, HIGH);
        receiveByte <<= 1;
        if (digitalRead(_sda_pin)) receiveByte |= 0x01;
        delay();
    }
    digitalWrite(_scl_pin, LOW);
    return receiveByte;
}

// ACK/NACK处理
uint8_t SoftI2C::waitAck(void) {
    uint8_t errTime = 0;
    sdaIn();
    digitalWrite(_sda_pin, HIGH);  // 释放总线
    delay();
    digitalWrite(_scl_pin, HIGH);
    delay();
    
    while (digitalRead(_sda_pin)) {
        if (++errTime > 250) {
            stop();
            return 1;
        }
    }
    digitalWrite(_scl_pin, LOW);
    delay();
    return 0;
}

void SoftI2C::ack(void) {
    digitalWrite(_scl_pin, LOW);
    sdaOut();
    digitalWrite(_sda_pin, LOW);
    delay();
    digitalWrite(_scl_pin, HIGH);
    delay();
    digitalWrite(_scl_pin, LOW);
    delay();
}

void SoftI2C::nack(void) {
    digitalWrite(_scl_pin, LOW);
    sdaOut();
    digitalWrite(_sda_pin, HIGH);
    delay();
    digitalWrite(_scl_pin, HIGH);
    delay();
    digitalWrite(_scl_pin, LOW);
    delay();
}

// 公开API
uint8_t SoftI2C::writeData(uint8_t slaveAddress, uint8_t* data, uint16_t len) {
    start();
    sendByte(slaveAddress << 1);  // 地址左移 + 写标志(0)
    if (waitAck()) return 1;
    
    for (uint8_t i = 0; i < len; i++) {
        sendByte(data[i]);
        if (waitAck()) return 1;
    }
    stop();
    return 0;
}

uint8_t SoftI2C::readData(uint8_t slaveAddress, uint8_t* data, uint8_t len) {
    start();
    sendByte((slaveAddress << 1) | 0x01);  // 地址左移 + 读标志(1)
    if (waitAck()) return 1;
    
    for (uint8_t i = 0; i < len; i++) {
        data[i] = readByte();
        if (i == len - 1) nack();
        else ack();
    }
    stop();
    return 0;
}