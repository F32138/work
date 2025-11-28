#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Uart.h"

int main(void)
{
    Uart::Config cfg_s9;
    Uart::Config cfg_s2;

    cfg_s9.device = "/dev/ttyS9";
    cfg_s2.device = "/dev/ttyS2";

    Uart uart_s9(cfg_s9);
    Uart uart_s2(cfg_s2);
    if (!uart_s2.open()) {
        printf("[UART] open failed\n");
        return -1;
    }
    if (!uart_s9.open()) {
        printf("[UART] open failed\n");
        return -1;
    }

    printf("[UART] opened %s\n", cfg_s9.device.c_str());
    printf("[UART] opened %s\n", cfg_s2.device.c_str());

    // 发送一些数据
    const uint8_t txData[] = { 0x11, 0x22, 0x33, 0x44 };
    int n = uart_s9.write(txData, (int)sizeof(txData));
    if (n < 0) {
        printf("[UART] write failed\n");
    } else {
        printf("[UART] TX %d bytes: [0x11 0x22 0x33 0x44]\n", n);
    }

    // 接收数据
    uint8_t rxBuf[256];
    memset(rxBuf, 0, sizeof(rxBuf));

    n = uart_s2.read(rxBuf, (int)sizeof(rxBuf), 500);
    if (n > 0) {
        int i;
        printf("[UART] RX %d bytes: [", n);
        for (i = 0; i < n; ++i) {
            printf("0x%02X", (unsigned int)rxBuf[i]);
            if (i + 1 < n) printf(" ");
        }
        printf("]\n");
    } else if (n == 0) {
        printf("[UART] no data (timeout)\n");
    } else {
        printf("[UART] read failed\n");
    }

    uart_s9.close();
    uart_s2.close();
    return 0;
}
