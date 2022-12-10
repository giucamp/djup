cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_executable(djup_test
	main.cpp
)

target_link_libraries(djup_test
	core djup)
