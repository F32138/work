#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "Gpio.h"

int main(void)
{
    // 配置GPIO为输出模式
    Gpio::Config cfg_output;
    cfg_output.pin = 143;  // 假设使用GPIO143作为输出
    cfg_output.direction = Gpio::Direction_Out;

    // 创建GPIO对象
    Gpio gpio_out(cfg_output);

    // 打开输出GPIO
    if (!gpio_out.open()) {
        printf("[GPIO] open output GPIO%d failed\n", cfg_output.pin);
        return -1;
    }
    printf("[GPIO] opened GPIO%d as OUTPUT\n", cfg_output.pin);

    // 测试输出功能：循环翻转电平
    printf("[GPIO] Testing output functionality...\n");
    for (int i = 0; i < 5; i++) {
        // 设置为高电平
        if (gpio_out.setValue(Gpio::Value_High)) {
            printf("[GPIO] GPIO%d set to HIGH\n", cfg_output.pin);
        } else {
            printf("[GPIO] Failed to set GPIO%d to HIGH\n", cfg_output.pin);
        }
        sleep(1);

        // 设置为低电平
        if (gpio_out.setValue(Gpio::Value_Low)) {
            printf("[GPIO] GPIO%d set to LOW\n", cfg_output.pin);
        } else {
            printf("[GPIO] Failed to set GPIO%d to LOW\n", cfg_output.pin);
        }
        sleep(1);

        printf("\n");
    }

    // 测试动态改变方向
    printf("[GPIO] Testing dynamic direction change...\n");
    if (gpio_out.setDirection(Gpio::Direction_In)) {
        printf("[GPIO] GPIO%d changed to INPUT mode\n", cfg_output.pin);
        
        int value = gpio_out.getValue();
        if (value >= 0) {
            printf("[GPIO] GPIO%d read value: %s\n", 
                   cfg_output.pin, 
                   (value == Gpio::Value_High) ? "HIGH" : "LOW");
        }
    } else {
        printf("[GPIO] Failed to change GPIO%d direction\n", cfg_output.pin);
    }

    // 关闭GPIO
    gpio_out.close();
    printf("[GPIO] GPIOs closed\n");

    return 0;
}

