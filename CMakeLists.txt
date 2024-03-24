cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME "ChargeDaemon")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 23)
add_executable(${PROJECT_NAME} Daemon/main.cpp)
add_executable(ChargeDispatch Dispatch/sendcommand.cpp)