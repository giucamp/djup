cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_library(djup STATIC
	private/alphabet.h
	private/builtin_names.h
	private/constant_shape.cpp
	private/constant_shape.h
	private/discrimination_network.cpp
	private/discrimination_network.h
	private/domain.cpp
	private/domain.h
	private/expression.cpp
	private/expression.h
	private/expression_to_string.cpp
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
	private/substitute_by_predicate.h
	private/tensor.cpp
	private/type.cpp
	private/type.h
	public/djup/tensor.h
	tests/test_djup.cpp
	tests/test_lexer.cpp
	tests/test_tensor_to_string.cpp
	tests/test_parse.cpp
	tests/test_pattern.cpp
	tests/test_type.cpp
)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)

target_link_libraries(djup core)