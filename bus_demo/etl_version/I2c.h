/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved
 *
 * File			: I2c.h
 * Author		: Fan Fei
 * Description	:
 * Comments		:
 * Date			: 2025-11-25
 *
 * Revision Control
 *  Ver | yyyy-mm-dd  | Who  | Description of changes
 * -----|-------------|------|--------------------------------------------
 * 		|			  |		 |
 * 		|			  |		 |
 **************************************************************************/
#pragma once

/***************************************************************************
 							include files
***************************************************************************/
#include <stdint.h>
#include "etl/string.h"

/***************************************************************************
 						macro definition
***************************************************************************/
#define I2C0_DEVICE "/dev/i2c0"
#define I2C1_DEVICE "/dev/i2c1"
#define I2C2_DEVICE "/dev/i2c2"
#define I2C3_DEVICE "/dev/i2c3"

/***************************************************************************
 						class declaration
***************************************************************************/
class I2c {
public:
    struct Config {
        etl::string<32> device;     // 设备节点路径，如 "/dev/i2c-2"
        uint8_t addr;               // 从设备地址
    };

    I2c();
    I2c(const Config& cfg);
    ~I2c();

    bool open();
    void close();
    bool isOpen() const { return m_fd >= 0; }

    bool reconfigure(const Config& cfg);
    const Config& config() const { return m_cfg; }

    // 基本读写
    bool writeBytes(const uint8_t* data, uint16_t len);
    bool readBytes(uint8_t* buf, uint16_t len);

    // 寄存器读写
    bool writeReg8(uint8_t reg, uint8_t val);
    bool writeRegBlock(uint8_t reg, const uint8_t* data, uint16_t len);
    bool readReg8(uint8_t reg, uint8_t* val);
    bool readRegBlock(uint8_t reg, uint8_t* buf, uint16_t len);

private:
    int    m_fd;
    Config m_cfg;

    bool setSlaveAddress(uint8_t addr);
};
/******************************** FILE END ********************************/
