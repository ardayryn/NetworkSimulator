cmake_minimum_required(VERSION 3.10)
project(NetworkSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tüm .h ve .cpp dosyaları bu klasörün kendisinde (düz yapı, alt klasör yok)
file(GLOB SOURCES "*.cpp")

add_executable(network_sim ${SOURCES})