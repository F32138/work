/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved
 *
 * File			: Can.h
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
#define CAN0_DEVICE "can0"
#define CAN1_DEVICE "can1"

/***************************************************************************
 						class declaration
***************************************************************************/
class Can {
public:
    struct Config {
        char ifName[16];       // 接口名，如 "can0"
        int  loopback;         // 是否打开回环，0 关，非 0 开
        int  recvOwn;          // 是否接收自己发的帧
        int  recvTimeoutMs;    // receive() 等待帧的最长时间，（毫秒），<=0 表示一直等
    };

    struct Frame {
        uint32_t id;            // 标准帧 11 位 ID 或 扩展帧 29 位 ID
        int      isExtended;    // 0: 标准帧，非 0: 扩展帧
        int      isRTR;         // 1: RTR 帧
        uint8_t  dlc;           // 数据长度码，0-8
        uint8_t  data[8];       // payload 数据
    };

    Can();
    Can(const Config& cfg);
    ~Can();

    bool open();
    void close();
    bool isOpen() const { return m_fd >= 0; }

    bool reconfigure(const Config& cfg);
    const Config& config() const { return m_cfg; }

    bool send(const Frame& frame);
    bool receive(Frame& frame); // 超时/错误返回 false

    // 设置简单过滤器：id/mask
    bool setFilter(uint32_t id, uint32_t mask);

private:
    int    m_fd;
    Config m_cfg;

    bool applyOptions();
};
/******************************** FILE END ********************************/
