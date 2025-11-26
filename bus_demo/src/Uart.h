/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved
 *
 * File			: Uart.h
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

/***************************************************************************
 						macro definition
***************************************************************************/
#define UART2_DEVICE "/dev/ttyS2"
#define UART3_DEVICE "/dev/ttyS3"

/***************************************************************************
 						class declaration
***************************************************************************/
class Uart {
public:
    enum Parity {
        Parity_None = 0,
        Parity_Even = 1,
        Parity_Odd  = 2
    };

    struct Config {
        char  device[32];      // 串口设备路径，例如 "/dev/ttyS2"
        int   baudrate;        // 例如 115200
        uint8_t dataBits;      // 5/6/7/8
        uint8_t stopBits;      // 1 或 2
        Parity parity;         // 校验
        int   hardwareFlowControl; // 0: 关闭 RTS/CTS，非 0: 开启
        int   readTimeoutMs;       // read 超时时间，毫秒，<=0 表示永不超时
    };

    Uart();
    Uart(const Config& cfg);
    ~Uart();

    bool open();
    void close();
    bool isOpen() const { return m_fd >= 0; }

    bool reconfigure(const Config& cfg);
    const Config& config() const { return m_cfg; }

    // 返回写入的字节数，失败返回 -1
    int write(const uint8_t* data, int len);

    // 读取最多 maxLen 字节：
    // >0  : 实际读取的字节数
    //  0  : 超时（在 readTimeoutMs 内没有任何数据）
    // <0  : 失败
    int read(uint8_t* buf, int maxLen);

private:
    int    m_fd;
    Config m_cfg;

    bool   applyTermios();
    int    baudToConstant(int baud); // 返回 B115200 等宏，对应的 int
};
/******************************** FILE END ********************************/
