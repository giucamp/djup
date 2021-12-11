cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_library(djup STATIC

)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)

TARGET_LINK_LIBRARIES(djup pthread)