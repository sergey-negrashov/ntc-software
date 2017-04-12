cmake_minimum_required(VERSION 3.0)
project(drs_ntc)

set(CMAKE_CXX_FLAGS "-std=c++11 -O2 -fno-strict-aliasing -Iinclude -Inet -I/usr/local/include -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10")
set(CMAKE_C_FLAGS "-O2 -fno-strict-aliasing -Iinclude -I/usr/local/include -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/net)
link_directories(/usr/local/lib)
include_directories(/usr/local/include)
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
        src/strlcpy.c
        src/motor.cpp 
	include/motor.hpp)

add_executable(drs_ntc ${SOURCE_FILES})
target_link_libraries (drs_ntc usb-1.0)
target_link_libraries (drs_ntc util)
target_link_libraries (drs_ntc pthread)
target_link_libraries (drs_ntc protobuf)
target_link_libraries (drs_ntc zmq)
target_link_libraries (drs_ntc newt)
