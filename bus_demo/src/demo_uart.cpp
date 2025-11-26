#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Uart.h"

int main(void)
{
    Uart::Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    // 串口设备名
    strcpy(cfg.device, UART2_DEVICE);   // 根据实际情况改，比如 "/dev/ttyS0"
    cfg.baudrate = 115200;
    cfg.dataBits = 8;
    cfg.stopBits = 1;
    cfg.parity   = Uart::Parity_None;
    cfg.hardwareFlowControl = 0;
    cfg.readTimeoutMs       = 500;      // 读超时 500 ms

    Uart uart(cfg);

    if (!uart.open()) {
        printf("[UART] open failed\n");
        return -1;
    }

    printf("[UART] opened %s\n", cfg.device);

    // 发送一些数据
    const uint8_t txData[] = { 0x11, 0x22, 0x33, 0x44 };
    int n = uart.write(txData, (int)sizeof(txData));
    if (n < 0) {
        printf("[UART] write failed\n");
    } else {
        printf("[UART] TX %d bytes: [0x11 0x22 0x33 0x44]\n", n);
    }

    // 接收数据
    uint8_t rxBuf[256];
    memset(rxBuf, 0, sizeof(rxBuf));

    n = uart.read(rxBuf, (int)sizeof(rxBuf));
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

    uart.close();
    return 0;
}
