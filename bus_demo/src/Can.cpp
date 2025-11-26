/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved
 *
 * File			: Can.cpp
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

/***************************************************************************
 							include files
***************************************************************************/
#include "Can.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/select.h>

/***************************************************************************
 						class definition
***************************************************************************/
Can::Can()
{
    m_fd = -1;
    memset(&m_cfg, 0, sizeof(m_cfg));
}

Can::Can(const Config& cfg)
{
    m_fd  = -1;
    m_cfg = cfg;
}

Can::~Can()
{
    close();
}

bool Can::applyOptions()
{
    if (m_fd < 0)
        return false;

    int loopback = m_cfg.loopback ? 1 : 0;
    if (setsockopt(m_fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK,
                   &loopback, sizeof(loopback)) < 0) {
        perror("setsockopt CAN_RAW_LOOPBACK");
        return false;
    }

    int recvOwn = m_cfg.recvOwn ? 1 : 0;
    if (setsockopt(m_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
                   &recvOwn, sizeof(recvOwn)) < 0) {
        perror("setsockopt CAN_RAW_RECV_OWN_MSGS");
        return false;
    }

    // 默认不过滤（全接收）
    struct can_filter filter;
    filter.can_id   = 0;
    filter.can_mask = 0;
    if (setsockopt(m_fd, SOL_CAN_RAW, CAN_RAW_FILTER,
                   &filter, sizeof(filter)) < 0) {
        perror("setsockopt CAN_RAW_FILTER");
        return false;
    }

    return true;
}

bool Can::open()
{
    if (isOpen())
        return true;

    m_fd = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_fd < 0) {
        perror("socket CAN_RAW");
        return false;
    }

    if (!applyOptions()) {
        close();
        return false;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    // 拷贝接口名
    strncpy(ifr.ifr_name, m_cfg.ifName, sizeof(ifr.ifr_name) - 1);
    ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = '\0';

    if (ioctl(m_fd, SIOCGIFINDEX, &ifr) < 0) { // ioctl(SIOCGIFINDEX) 把 "can0" 转成内核的接口索引号 ifr.ifr_ifindex
        perror("ioctl SIOCGIFINDEX");
        close();
        return false;
    }

    struct sockaddr_can addr;
    memset(&addr, 0, sizeof(addr));
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (::bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind can");
        close();
        return false;
    }

    return true;
}

void Can::close()
{
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool Can::reconfigure(const Config& cfg)
{
    m_cfg = cfg;

    if (!isOpen())
        return true;

    // 为简单起见，重开 socket
    close();
    return open();
}

bool Can::setFilter(uint32_t id, uint32_t mask)
{
    if (m_fd < 0)
        return false;

    struct can_filter filter;
    filter.can_id   = id;
    filter.can_mask = mask;

    if (setsockopt(m_fd, SOL_CAN_RAW, CAN_RAW_FILTER,
                   &filter, sizeof(filter)) < 0) {
        perror("setsockopt CAN_RAW_FILTER");
        return false;
    }
    return true;
}

bool Can::send(const Frame& frame)
{
    if (m_fd < 0)
        return false;

    struct can_frame cf;
    memset(&cf, 0, sizeof(cf));

    if (frame.isExtended) {
        cf.can_id = frame.id | CAN_EFF_FLAG;
    } else {
        cf.can_id = frame.id & CAN_SFF_MASK;
    }

    if (frame.isRTR) {
        cf.can_id |= CAN_RTR_FLAG;
    }

    cf.can_dlc = frame.dlc;
    if (cf.can_dlc > 8)
        cf.can_dlc = 8;

    memcpy(cf.data, frame.data, cf.can_dlc);

    int n = (int)::write(m_fd, &cf, sizeof(cf));
    if (n < 0) {
        perror("can write");
        return false;
    }
    return (n == (int)sizeof(cf));
}

bool Can::receive(Frame& frame)
{
    if (m_fd < 0)
        return false;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_fd, &readfds);

    struct timeval tv;
    struct timeval* ptv = nullptr;

    if (m_cfg.recvTimeoutMs > 0) {
        tv.tv_sec  = m_cfg.recvTimeoutMs / 1000;
        tv.tv_usec = (m_cfg.recvTimeoutMs % 1000) * 1000;
        ptv = &tv;
    }

    int ret = ::select(m_fd + 1, &readfds, nullptr, nullptr, ptv);
    if (ret < 0) {
        perror("select can");
        return false;
    } else if (ret == 0) {
        // 超时
        return false;
    }

    struct can_frame cf;
    int n = (int)::read(m_fd, &cf, sizeof(cf));
    if (n < 0) {
        perror("can read");
        return false;
    }
    if (n != (int)sizeof(cf)) {
        return false;
    }

    frame.isExtended = (cf.can_id & CAN_EFF_FLAG) ? 1 : 0;
    frame.isRTR      = (cf.can_id & CAN_RTR_FLAG) ? 1 : 0;

    if (frame.isExtended) {
        frame.id = cf.can_id & CAN_EFF_MASK;
    } else {
        frame.id = cf.can_id & CAN_SFF_MASK;
    }

    frame.dlc = cf.can_dlc;
    if (frame.dlc > 8)
        frame.dlc = 8;

    memcpy(frame.data, cf.data, frame.dlc);

    return true;
}
/******************************** FILE END ********************************/
