cmake_minimum_required(VERSION 3.7)
project(readoutd2)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        lib/common.hpp
        lib/coredataqueue.hpp
        lib/netutil.h
        lib/packets.hpp
        lib/readoutProtocol.hpp
        lib/registerinterface.hpp
        lib/settings.hpp
        lib/storage.hpp
        lib/trigger.hpp
        lib/udplistner.hpp
        lib/web++.hpp
        lib/monitor.hpp
        lib/motor.hpp
        src/registerinterface.cpp
        src/settings.cpp
        src/storage.cpp
        src/trigger.cpp
        src/udplistner.cpp
        src/udpmtc.cpp
        src/monitor.cpp
        src/motor.cpp)

add_executable(readoutd2 ${SOURCE_FILES})
target_include_directories(readoutd2 PUBLIC ".")
target_link_libraries(readoutd2 boost_system boost_thread boost_filesystem mtc-data newt)