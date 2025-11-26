#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Can.h"

int main(void)
{
    Can::Config cfg;
    memset(&cfg, 0, sizeof(cfg));

    // CAN 接口名，根据系统中 ip link 看实际是TODO:can0 还是 can1
    cfg.ifName        = CAN0_DEVICE;
    cfg.loopback      = 0;     // 测试自环可以设为 1
    cfg.recvOwn       = 0;     // 是否接收自己发的帧
    cfg.recvTimeoutMs = 500;   // 接收超时 500 ms

    Can can(cfg);

    if (!can.open()) {
        printf("[CAN] open failed\n");
        return -1;
    }

    printf("[CAN] opened %s\n", cfg.ifName);

    // 发送一帧
    Can::Frame tx;
    memset(&tx, 0, sizeof(tx));
    tx.id         = 0x123;
    tx.isExtended = 0;
    tx.isRTR      = 0;
    tx.dlc        = 2;
    tx.data[0]    = 0x11;
    tx.data[1]    = 0x22;

    if (can.send(tx)) {
        int i;
        printf("[CAN] TX id=0x%X dlc=%u data=[",
               (unsigned int)tx.id,
               (unsigned int)tx.dlc);
        for (i = 0; i < tx.dlc; ++i) {
            printf("0x%02X", (unsigned int)tx.data[i]);
            if (i + 1 < tx.dlc) printf(" ");
        }
        printf("]\n");
    } else {
        printf("[CAN] send failed\n");
    }

    // 接收一帧（带超时）
    Can::Frame rx;
    memset(&rx, 0, sizeof(rx));

    if (can.receive(rx)) {
        int i;
        printf("[CAN] RX id=0x%X %s %s dlc=%u data=[",
               (unsigned int)rx.id,
               rx.isExtended ? "(EXT)" : "(STD)",
               rx.isRTR      ? "RTR"   : "DATA",
               (unsigned int)rx.dlc);
        for (i = 0; i < rx.dlc; ++i) {
            printf("0x%02X", (unsigned int)rx.data[i]);
            if (i + 1 < rx.dlc) printf(" ");
        }
        printf("]\n");
    } else {
        printf("[CAN] no frame received (timeout or error)\n");
    }

    can.close();
    return 0;
}
