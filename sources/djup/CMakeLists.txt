cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_library(djup STATIC
	private/alphabet.h
	private/discrimination_network.cpp
	private/discrimination_network.h
	private/domain.cpp
	private/domain.h
	private/expression.cpp
	private/expression.h
	private/fixed_shape.cpp
	private/fixed_shape.h
	private/indices.cpp
	private/indices.h
	private/lexer.cpp
	private/lexer.h
	private/name.cpp
	private/name.h
	private/parser.cpp
	private/parser.h
	private/scope.cpp
	private/scope.h
	private/tensor.cpp
	private/type.cpp
	private/type.h
	public/djup/tensor.h
	tests/test_core.cpp
	tests/test_from_chars.cpp
	tests/test_queue.cpp
	tests/test_split.cpp
	tests/test_to_chars.cpp
	tests/test_to_string.cpp
	tests/test_traits.cpp
	tests/test_udp_socket.cpp
)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)

target_link_libraries(djup core)