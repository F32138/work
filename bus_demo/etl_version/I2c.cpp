/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved.
 *
 * File			: I2c.cpp
 * Author		: Fan Fei
 * Description	:
 * Date			: 2025-11-25
 *
 * Revision Control
 *  Ver | yyyy-mm-dd  | Who  | Description of changes
 * -----|-------------|------|--------------------------------------------
 * 		|			  |		 |
***************************************************************************/

/***************************************************************************
 							include files
***************************************************************************/
#include "I2c.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

/***************************************************************************
 						class definition
***************************************************************************/
I2c::I2c()
{
    m_fd = -1;
    memset(&m_cfg, 0, sizeof(m_cfg));
}

I2c::I2c(const Config& cfg)
{
    m_fd  = -1;
    m_cfg = cfg;
}

I2c::~I2c()
{
    close();
}

bool I2c::open()
{
    if (isOpen())
        return true;

    m_fd = ::open(m_cfg.device.c_str(), O_RDWR);
    if (m_fd < 0) {
        perror("open i2c");
        return false;
    }

    if (!setSlaveAddress(m_cfg.addr)) {
        close();
        return false;
    }

    return true;
}

void I2c::close()
{
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool I2c::reconfigure(const Config& cfg)
{
    // 直接拷贝配置
    m_cfg = cfg;

    if (isOpen()) {
        // 目前只需要重新设置从地址
        return setSlaveAddress(m_cfg.addr);
    }
    return true;
}

bool I2c::setSlaveAddress(uint8_t addr)
{
    if (m_fd < 0)
        return false;

    if (ioctl(m_fd, I2C_SLAVE, addr) < 0) { // I2C_SLAVE 指定从设备地址
        perror("ioctl I2C_SLAVE");
        return false;
    }
    return true;
}

bool I2c::writeBytes(const uint8_t* data, uint16_t len)
{
    if (m_fd < 0)
        return false;
    if (data == 0 || len == 0)
        return false;

    int ret = ::write(m_fd, data, len);
    if (ret < 0) {
        perror("i2c write");
        return false;
    }
    return (ret == (int)len);
}

bool I2c::readBytes(uint8_t* buf, uint16_t len)
{
    if (m_fd < 0)
        return false;
    if (buf == 0 || len == 0)
        return false;

    int ret = ::read(m_fd, buf, len);
    if (ret < 0) {
        perror("i2c read");
        return false;
    }
    return (ret == (int)len);
}

bool I2c::writeReg8(uint8_t reg, uint8_t val)
{
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = val;
    return writeBytes(buf, 2);
}

bool I2c::writeRegBlock(uint8_t reg, const uint8_t* data, uint16_t len)
{
    if (data == 0 || len == 0)
        return false;

    // 根据最大寄存器块大小调整这个 buffer 尺寸
    uint8_t buf[256];
    if ((uint16_t)(len + 1) > (uint16_t)sizeof(buf))
        return false;

    buf[0] = reg;
    memcpy(&buf[1], data, len);

    return writeBytes(buf, len + 1);
}

bool I2c::readReg8(uint8_t reg, uint8_t* val)
{
    if (val == 0)
        return false;

    if (!writeBytes(&reg, 1))
        return false;

    return readBytes(val, 1);
}

bool I2c::readRegBlock(uint8_t reg, uint8_t* buf, uint16_t len)
{
    if (buf == 0 || len == 0)
        return false;

    if (!writeBytes(&reg, 1))
        return false;

    return readBytes(buf, len);
}
/******************************** FILE END ********************************/
