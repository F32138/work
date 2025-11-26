cmake_minimum_required(VERSION 3.10)
project(bus_demo CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -O2)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(BUS_SOURCES
    src/Uart.cpp
    src/I2c.cpp
    src/Can.cpp
)

# UART demo
add_executable(uart_demo
    src/demo_uart.cpp
    ${BUS_SOURCES}
)

# I2C demo
add_executable(i2c_demo
    src/demo_i2c.cpp
    ${BUS_SOURCES}
)

# CAN demo
add_executable(can_demo
    src/demo_can.cpp
    ${BUS_SOURCES}
)
