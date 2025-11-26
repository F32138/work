#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "I2c.h"

int main(void)
{
    I2c::Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    // 根据实际硬件修改总线号
    cfg.device = I2C0_DEVICE;     // 比如 "/dev/i2c0"
    cfg.addr = 0x48;              // 从设备地址

    I2c i2c(cfg);

    if (!i2c.open()) {
        printf("[I2C] open failed\n");
        return -1;
    }

    printf("[I2C] opened %s, addr=0x%02X\n",
           cfg.device.c_str(), (unsigned int)cfg.addr);

    // 示例：读取寄存器 0x00
    uint8_t reg  = 0x00;
    uint8_t value = 0;

    if (i2c.readReg8(reg, &value)) {
        printf("[I2C] Read reg 0x%02X = 0x%02X\n",
               (unsigned int)reg, (unsigned int)value);
    } else {
        printf("[I2C] readReg8 failed\n");
    }

    i2c.close();
    return 0;
}
