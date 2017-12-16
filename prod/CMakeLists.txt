cmake_minimum_required(VERSION 3.8)
project(udp)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES server.cpp utils.h)

#LINK_DIRECTORIES(spdlog)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -s -O3 -I C:/Users/Shiro/Desktop/asio-1.10.6/include")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -static")
#link_libraries(ws2_32 wsock32)


set(CMAKE_CXX_FLAGS "-fpermissive")

add_executable(client client.cpp utils.h)
TARGET_LINK_LIBRARIES(client pthread)

add_executable(udp ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(udp pthread)


#link_libraries(ws2_32 wsock32)
#TARGET_LINK_LIBRARIES(tcp libws2_32.a)
#TARGET_LINK_LIBRARIES(tcp libmswsock.a)
#TARGET_LINK_LIBRARIES(tcp pthread)

#C:\MinGW\include\winsock2.h