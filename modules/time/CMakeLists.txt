cmake_minimum_required(VERSION 3.9)
project(time)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        /usr/local/BM/dev/include
        /usr/local/BM/dev/
        /usr/local/BM/dev/OSPlatformUtil/src/osplatformutil.h
)
LINK_DIRECTORIES(/usr/local/BM/dev/lib/)
add_library(time SHARED library.cpp library.h)
TARGET_LINK_LIBRARIES(time BerryMath)