cmake_minimum_required(VERSION 3.6)
project(drs_ntc)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -Iinclude -I/usr/local/include -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCE_FILES
    include/averager.h
    include/DRS.h
    include/musbstd.h
    include/mxml.h
    include/strlcpy.h
    src/averager.cpp
    src/DRS.cpp
    src/drs_ntc.cpp
    src/musbstd.c
    src/mxml.c
    src/strlcpy.c)

add_executable(drs_ntc ${SOURCE_FILES})
target_link_libraries (drs_ntc usb-1.0)