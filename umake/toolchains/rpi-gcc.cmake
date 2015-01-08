set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
#set(CMAKE_SYSTEM_PROCESSOR arm)

set (RPI_TOOLCHAIN ON)
set (RPI_TOOLCHAIN_GCC ON)
set (RPI_TOOLCHAIN_PREFIX "arm-unknown-linux-gnueabi")
set (RPI_TOOLCHAIN_ROOT "/Volumes/CrossToolNG/x-tools/arm-unknown-linux-gnueabi")

# specify the cross compiler
set(CMAKE_C_COMPILER   ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${RPI_TOOLCHAIN_ROOT}/bin/${RPI_TOOLCHAIN_PREFIX}-g++)

# where is the target environment 
set(CMAKE_FIND_ROOT_PATH  ${RPI_TOOLCHAIN_ROOT})

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
