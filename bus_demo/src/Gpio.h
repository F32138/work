/***************************************************************************
 * CopyRight(c)		YoPlore	, All rights reserved
 *
 * File			: Gpio.h
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
#include "etl/string.h"

/***************************************************************************
 						macro definition
***************************************************************************/
#define GPIO_EXPORT_PATH "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH "/sys/class/gpio/unexport"
#define GPIO_BASE_PATH "/sys/class/gpio/gpio"

/***************************************************************************
 						class declaration
***************************************************************************/
class Gpio {
public:
    enum Direction {
        Direction_In  = 0,  // 输入
        Direction_Out = 1   // 输出
    };

    enum Value {
        Value_Low  = 0,     // 低电平
        Value_High = 1      // 高电平
    };

    struct Config {
        Config()
            : pin(0)
            , direction(Direction_In)
        {
        }

        int       pin;       // GPIO引脚编号
        Direction direction; // 方向：输入或输出
    };

    Gpio();
    Gpio(const Config& cfg);
    ~Gpio();

    bool open();
    void close();
    bool isOpen() const { return m_pin >= 0; }

    bool reconfigure(const Config& cfg);

    // 设置方向（输入/输出）
    bool setDirection(Direction dir);

    // 设置输出值（仅输出模式有效）
    // 返回 true 成功，false 失败
    bool setValue(Value val);

    // 读取输入值（仅输入模式有效）
    // 返回 Value_Low 或 Value_High，失败返回 -1
    int getValue();

    // 获取当前配置
    const Config& config() const { return m_cfg; }

private:
    int    m_pin;        // GPIO引脚编号，-1表示未打开
    int    m_valueFd;    // value文件描述符
    Config m_cfg;
    bool   m_exportedByUs; // 标记是否由我们导出的GPIO

    // 内部辅助函数
    bool exportPin(int pin, bool* wasExported);
    bool unexportPin(int pin);
    bool setDirectionInternal(Direction dir);
    bool openValueFile();
    void closeValueFile();
    bool checkPinFilesReady(int pin); // 检查GPIO文件是否就绪
    etl::string<64> getPinPath(int pin, const char* file);
};

/******************************** FILE END ********************************/
