/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved
 *
 * File			: Gpio.cpp
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
#include "Gpio.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

/***************************************************************************
 						class definition
***************************************************************************/
Gpio::Gpio()
    : m_pin(-1)
    , m_valueFd(-1)
    , m_cfg()
    , m_exportedByUs(false)
{
}

Gpio::Gpio(const Config& cfg)
    : m_pin(-1)
    , m_valueFd(-1)
    , m_cfg(cfg)
    , m_exportedByUs(false)
{
}

Gpio::~Gpio()
{
    close();
}

bool Gpio::checkPinFilesReady(int pin)
{
    etl::string<64> directionPath = getPinPath(pin, "direction");
    etl::string<64> valuePath = getPinPath(pin, "value");
    
    struct stat st;
    if (stat(directionPath.c_str(), &st) != 0 || 
        stat(valuePath.c_str(), &st) != 0) {
        return false;
    }
    
    // 文件存在，尝试打开一次确认可访问
    int testFd = ::open(directionPath.c_str(), O_RDONLY);
    if (testFd >= 0) {
        ::close(testFd);
        return true;
    }
    
    return false;
}

bool Gpio::exportPin(int pin, bool* wasExported)
{
    if (wasExported) {
        *wasExported = false;
    }

    char pinStr[16];
    snprintf(pinStr, sizeof(pinStr), "%d", pin);

    int fd = ::open(GPIO_EXPORT_PATH, O_WRONLY);
    if (fd < 0) {
        perror("open gpio export");
        return false;
    }

    ssize_t len = ::write(fd, pinStr, strlen(pinStr));
    ::close(fd);

    if (len < 0) {
        // 如果GPIO已经导出，errno会是EBUSY或EEXIST
        if (errno == EBUSY || errno == EEXIST) {
            if (wasExported) {
                *wasExported = true;
            }
            // 检查文件是否已经就绪
            if (checkPinFilesReady(pin)) {
                return true;
            }
            // 文件存在但可能还在初始化，等待一下
            usleep(100000); // 100ms
            return checkPinFilesReady(pin);
        } else {
            perror("write gpio export");
            return false;
        }
    }

    // 新导出的GPIO，等待文件创建完成
    // 使用指数退避策略，最多等待约1秒
    int retries = 20;
    int delay = 10000; // 初始10ms
    while (retries > 0) {
        usleep(delay);
        
        if (checkPinFilesReady(pin)) {
            return true;
        }
        
        retries--;
        if (delay < 50000) {
            delay += 5000; // 逐渐增加延迟
        }
    }

    fprintf(stderr, "gpio export: direction/value files not ready after export\n");
    return false;
}

bool Gpio::unexportPin(int pin)
{
    char pinStr[16];
    snprintf(pinStr, sizeof(pinStr), "%d", pin);

    int fd = ::open(GPIO_UNEXPORT_PATH, O_WRONLY);
    if (fd < 0) {
        perror("open gpio unexport");
        return false;
    }

    ssize_t len = ::write(fd, pinStr, strlen(pinStr));
    ::close(fd);

    if (len < 0 && errno != ENOENT) { // ENOENT表示GPIO未导出，可以忽略
        perror("write gpio unexport");
        return false;
    }

    return true;
}

etl::string<64> Gpio::getPinPath(int pin, const char* file)
{
    etl::string<64> path;
    path = GPIO_BASE_PATH;
    char pinStr[16];
    snprintf(pinStr, sizeof(pinStr), "%d", pin);
    path += pinStr;
    path += "/";
    path += file;
    return path;
}

bool Gpio::setDirectionInternal(Direction dir)
{
    if (m_pin < 0)
        return false;

    etl::string<64> directionPath = getPinPath(m_pin, "direction");

    int fd = ::open(directionPath.c_str(), O_WRONLY);
    if (fd < 0) {
        perror("open gpio direction");
        return false;
    }

    const char* dirStr = (dir == Direction_In) ? "in" : "out";
    ssize_t len = ::write(fd, dirStr, strlen(dirStr));
    ::close(fd);

    if (len < 0) {
        perror("write gpio direction");
        return false;
    }

    return true;
}

bool Gpio::openValueFile()
{
    if (m_pin < 0)
        return false;

    closeValueFile(); // 确保之前已关闭

    etl::string<64> valuePath = getPinPath(m_pin, "value");

    m_valueFd = ::open(valuePath.c_str(), O_RDWR);
    if (m_valueFd < 0) {
        perror("open gpio value");
        return false;
    }

    return true;
}

void Gpio::closeValueFile()
{
    if (m_valueFd >= 0) {
        ::close(m_valueFd);
        m_valueFd = -1;
    }
}

bool Gpio::open()
{
    if (isOpen())
        return true;

    if (m_cfg.pin < 0) {
        fprintf(stderr, "gpio pin number is invalid\n");
        return false;
    }

    // 导出GPIO，并记录是否由我们导出
    bool wasExported = false;
    if (!exportPin(m_cfg.pin, &wasExported)) {
        return false;
    }

    m_pin = m_cfg.pin;
    m_exportedByUs = !wasExported; // 如果不是已导出的，说明是我们导出的

    // 设置方向
    if (!setDirectionInternal(m_cfg.direction)) {
        close();
        return false;
    }

    // 打开value文件
    if (!openValueFile()) {
        close();
        return false;
    }

    return true;
}

void Gpio::close()
{
    closeValueFile();

    if (m_pin >= 0) {
        // 只有当我们导出的GPIO才取消导出
        if (m_exportedByUs) {
            unexportPin(m_pin);
            m_exportedByUs = false;
        }
        m_pin = -1;
    }
}

bool Gpio::reconfigure(const Config& cfg)
{
    bool wasOpen = isOpen();
    int oldPin = m_pin;
    Direction oldDir = m_cfg.direction;

    if (wasOpen) {
        close();
    }

    m_cfg = cfg;

    if (wasOpen) {
        // 如果引脚号改变，需要重新打开
        if (oldPin != m_cfg.pin) {
            return open();
        }
        
        // 引脚号相同，只需要重新打开并设置方向
        bool wasExported = false;
        if (!exportPin(m_cfg.pin, &wasExported)) {
            return false;
        }
        
        m_pin = m_cfg.pin;
        m_exportedByUs = !wasExported;
        
        // 设置方向（如果改变）
        if (oldDir != m_cfg.direction) {
            if (!setDirectionInternal(m_cfg.direction)) {
                close();
                return false;
            }
            // 方向改变时，需要重新打开value文件以确保权限正确
            if (!openValueFile()) {
                close();
                return false;
            }
        } else {
            // 方向未改变，只需重新打开value文件
            if (!openValueFile()) {
                close();
                return false;
            }
        }
    }

    return true;
}

bool Gpio::setDirection(Direction dir)
{
    if (m_pin < 0)
        return false;

    // 如果方向没有改变，不需要做任何操作
    if (m_cfg.direction == dir) {
        return true;
    }

    if (!setDirectionInternal(dir)) {
        return false;
    }

    m_cfg.direction = dir;

    // 方向改变时，需要重新打开value文件以确保权限正确
    // 因为输入和输出可能需要不同的文件访问模式
    if (!openValueFile()) {
        return false;
    }

    return true;
}

bool Gpio::setValue(Value val)
{
    if (m_valueFd < 0)
        return false;

    if (m_cfg.direction != Direction_Out) {
        fprintf(stderr, "gpio is not configured as output\n");
        return false;
    }

    const char* valStr = (val == Value_Low) ? "0" : "1";
    ssize_t len = ::write(m_valueFd, valStr, 1);
    
    if (len < 0) {
        perror("gpio write value");
        return false;
    }

    // 确保数据写入
    if (fsync(m_valueFd) < 0) {
        perror("gpio fsync");
        // 不返回false，因为可能在某些系统上fsync不可用
    }

    return true;
}

int Gpio::getValue()
{
    if (m_valueFd < 0)
        return -1;

    if (m_cfg.direction != Direction_In) {
        fprintf(stderr, "gpio is not configured as input\n");
        return -1;
    }

    // 将文件指针重置到开头
    if (lseek(m_valueFd, 0, SEEK_SET) < 0) {
        perror("gpio lseek");
        return -1;
    }

    char buf[4];
    ssize_t len = ::read(m_valueFd, buf, sizeof(buf) - 1);
    
    if (len < 0) {
        perror("gpio read value");
        return -1;
    }

    if (len == 0) {
        fprintf(stderr, "gpio read value: no data\n");
        return -1;
    }

    if (buf[0] == '1') {
        return Value_High;
    } else if (buf[0] == '0') {
        return Value_Low;
    }
    
    fprintf(stderr, "gpio read value: invalid first character '%c'\n", buf[0]);
    return -1;
}

/******************************** FILE END ********************************/
