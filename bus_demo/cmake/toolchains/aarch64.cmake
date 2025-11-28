set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TOOLCHAIN_ROOT "/home/tronlong/RK3562/Ubuntu/rk3562-ubuntu20.04-sdk-v1.0/sysroots/x86_64-linux/bin/gcc-linaro-10.2.1-2021.01-x86_64_aarch64-linux-gnu")
set(TOOLCHAIN_BIN "${TOOLCHAIN_ROOT}/bin")

set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN}/aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN}/aarch64-linux-gnu-g++")
set(CMAKE_AR "${TOOLCHAIN_BIN}/aarch64-linux-gnu-ar")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN}/aarch64-linux-gnu-ranlib")

set(CMAKE_SYSROOT "/home/tronlong/RK3562/Ubuntu/rk3562-ubuntu20.04-sdk-v1.0/sysroots/armv8a-linux")

set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

