cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_library(core STATIC
	private/diagnostic.cpp
	private/from_chars.cpp
	private/has_fp_charconv.h
	private/sockets.cpp
	private/to_chars.cpp
	private/to_string.cpp
	public/core/address.h
	public/core/algorithms.h
	public/core/diagnostic.h
	public/core/flags.h
	public/core/from_chars.h
	public/core/hash.h
	public/core/hash_variant.h
	public/core/numeric_cast.h
	public/core/pointer_iterator.h
	public/core/queue.h
	public/core/sockets.h
	public/core/span.h
	public/core/split.h
	public/core/bits.h
	public/core/to_chars.h
	public/core/to_string.h
	public/core/traits.h
	tests/test_core.cpp
	tests/test_from_chars.cpp
	tests/test_queue.cpp
	tests/test_split.cpp
	tests/test_to_chars.cpp
	tests/test_to_string.cpp
	tests/test_bits.cpp
	tests/test_traits.cpp
	tests/test_udp_socket.cpp
)

target_include_directories(core PUBLIC public)
target_include_directories(core PRIVATE .)

TARGET_LINK_LIBRARIES(core pthread)