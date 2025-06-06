cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4)
endif()

add_library(core STATIC

    #headers
    private/has_fp_charconv.h
    public/core/address.h
    public/core/algorithms.h
    public/core/bits.h
    public/core/diagnostic.h
    public/core/flags.h
    public/core/from_chars.h
    public/core/graph_wiz.h
    public/core/hash.h
    public/core/hash_variant.h
    public/core/immutable_vector.h
    public/core/memory.h
    public/core/name.h
    public/core/numeric_cast.h
    public/core/pointer_iterator.h
    public/core/pool.h
    public/core/sockets.h
    public/core/span.h
    public/core/split.h
    public/core/system_utils.h
    public/core/to_chars.h
    public/core/to_string.h
    public/core/traits.h

    #cpps
    private/diagnostic.cpp
    private/from_chars.cpp
    private/graph_wiz.cpp
    private/name.cpp
    private/sockets.cpp
    private/system_utils.cpp
    private/to_chars.cpp
    private/to_string.cpp
    tests/test_algorithm.cpp
    tests/test_bits.cpp
    tests/test_core.cpp
    tests/test_from_chars.cpp
    tests/test_graph_wiz.cpp
    tests/test_immutable_vector.cpp
    tests/test_memory.cpp
    tests/test_pool.cpp
    tests/test_split.cpp
    tests/test_system_utils.cpp
    tests/test_to_chars.cpp
    tests/test_to_string.cpp
    tests/test_traits.cpp
    tests/test_udp_socket.cpp
)

target_include_directories(core PUBLIC public)
target_include_directories(core PRIVATE .)

TARGET_LINK_LIBRARIES(core pthread)