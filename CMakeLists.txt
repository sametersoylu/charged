cmake_minimum_required(VERSION 3.10)
set(PROJECT_NAME "ChargeDaemon")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 23)
add_executable(charger Daemon/main.cpp)
add_executable(charged Dispatch/sendcommand.cpp)