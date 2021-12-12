cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_executable(test
	main.cpp
)

target_link_libraries(test
	core djup)
