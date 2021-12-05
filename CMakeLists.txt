cmake_minimum_required(VERSION 3.9)
project(djup)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-invalid-offsetof -Wno-abi")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++ -lpthread")
endif()

add_subdirectory(sources/core)
add_subdirectory(sources/test)

