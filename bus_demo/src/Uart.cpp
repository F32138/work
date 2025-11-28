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
#include "Uart.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/select.h>

/***************************************************************************
 						class definition
***************************************************************************/
Uart::Uart()
    : m_fd(-1)
    , m_cfg()
{
}

Uart::Uart(const Config& cfg)
    : m_fd(-1)
    , m_cfg(cfg)
{
}

Uart::~Uart()
{
    close();
}

int Uart::baudToConstant(int baud)
{
    switch (baud) {
    case 1200:   return B1200;
    case 2400:   return B2400;
    case 4800:   return B4800;
    case 9600:   return B9600;
    case 19200:  return B19200;
    case 38400:  return B38400;
    case 57600:  return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
#ifdef B460800
    case 460800: return B460800;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
    default:
        return B115200; // 默认
    }
}

bool Uart::applyTermios()
{
    if (m_fd < 0)
        return false;

    struct termios tio;
    if (tcgetattr(m_fd, &tio) != 0) {
        perror("tcgetattr");
        return false;
    }

    // 设置波特率
    const int speed = baudToConstant(m_cfg.baudrate);
    cfsetispeed(&tio, (speed_t)speed);
    cfsetospeed(&tio, (speed_t)speed);

    // 本地连接，允许接收
    tio.c_cflag |= (CLOCAL | CREAD);

    // 数据位
    tio.c_cflag &= ~CSIZE;
    switch (m_cfg.dataBits) {
    case 5: tio.c_cflag |= CS5; break;
    case 6: tio.c_cflag |= CS6; break;
    case 7: tio.c_cflag |= CS7; break;
    case 8:
    default: tio.c_cflag |= CS8; break;
    }

    // 停止位
    if (m_cfg.stopBits == 2) {
        tio.c_cflag |= CSTOPB;
    } else {
        tio.c_cflag &= ~CSTOPB;
    }

    // 校验
    tio.c_cflag &= ~(PARENB | PARODD);
    if (m_cfg.parity == Parity_Even) {
        tio.c_cflag |= PARENB;
        tio.c_cflag &= ~PARODD;
    } else if (m_cfg.parity == Parity_Odd) {
        tio.c_cflag |= PARENB;
        tio.c_cflag |= PARODD;
    }

#ifdef CRTSCTS
    if (m_cfg.hardwareFlowControl) {
        tio.c_cflag |= CRTSCTS;
    } else {
        tio.c_cflag &= ~CRTSCTS;
    }
#endif

    // 原始模式（不过滤、不翻译）
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;

    // 控制字符：非规范模式下，VMIN/VTIME 控制 read 行为
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 0;

    // 清空接收缓冲
    tcflush(m_fd, TCIFLUSH);

    if (tcsetattr(m_fd, TCSANOW, &tio) != 0) {
        perror("tcsetattr");
        return false;
    }

    return true;
}

bool Uart::open()
{
    if (isOpen())
        return true;

    if (m_cfg.device.empty()) {
        fprintf(stderr, "uart device is empty\n");
        return false;
    }

    // 非阻塞打开，避免被 modem 信号之类卡住
    m_fd = ::open(m_cfg.device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0) {
        perror("open uart");
        return false;
    }

    if (!applyTermios()) {
        close();
        return false;
    }

    // 切回阻塞模式（真正的超时交给 select 控）
    int flags = fcntl(m_fd, F_GETFL, 0);
    if (flags >= 0) {
        flags &= ~O_NONBLOCK;
        fcntl(m_fd, F_SETFL, flags);
    }

    return true;
}

void Uart::close()
{
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

bool Uart::reconfigure(const Config& cfg)
{
    m_cfg = cfg;

    if (!isOpen())
        return true;

    return applyTermios();
}

int Uart::write(const uint8_t* data, int len)
{
    if (m_fd < 0 || data == nullptr || len <= 0)
        return -1;

    int total = 0;
    while (total < len) {
        int ret = (int)::write(m_fd, data + total, (size_t)(len - total));
        if (ret < 0) {
            if (errno == EINTR)
                // 写过程中被信号中断，按 POSIX 约定重试即可
                continue;
            perror("uart write");
            return -1;
        }
        total += ret;
    }
    return total;
}

int Uart::read(uint8_t* buf, int maxLen, int readTimeoutMs)
{
    if (m_fd < 0 || buf == nullptr || maxLen <= 0)
        return -1;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_fd, &readfds);

    struct timeval tv;
    struct timeval* ptv = nullptr;

    if (readTimeoutMs > 0) {
        tv.tv_sec  = readTimeoutMs / 1000;
        tv.tv_usec = (readTimeoutMs % 1000) * 1000;
        ptv = &tv;
    }

    int ret = ::select(m_fd + 1, &readfds, nullptr, nullptr, ptv);
    if (ret < 0) {
        if (errno == EINTR)
            // select 同样可能因为信号退出，此时直接返回 0 表示未读到数据
            return 0;
        perror("select uart");
        return -1;
    } else if (ret == 0) {
        return 0;
    }

    int n = 0;
    while (n < maxLen) {
        ret = (int)::read(m_fd, buf + n, (size_t)(maxLen - n));
        if (ret < 0) {
            if (errno == EINTR)
                // read 被信号打断时继续读，避免把中断误报为错误
                continue;
            perror("uart read");
            return -1;
        }
        if (ret == 0) {
            // 已无更多数据，直接返回当前读取量
            break;
        }
        n += ret;
    }

    return n;
}
/******************************** FILE END ********************************/
