cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_library(djup STATIC
	private/alphabet.h
	private/builtin_names.h
	private/constant_shape.cpp
	private/constant_shape.h
	private/diagnostic.h
	private/discrimination_net.cpp
	private/discrimination_net.h
	private/expression.cpp
	private/expression.h
	private/expression_to_string.cpp
	private/indices.cpp
	private/indices.h
	private/is.cpp
	private/lexer.cpp
	private/lexer.h
	private/name.cpp
	private/name.h
	private/parser.cpp
	private/parser.h
	private/namespace.cpp
	private/namespace.h
	private/standard_scope.cpp
	private/standard_scope.h
	private/substitute_by_predicate.h
	private/tensor.cpp
	public/djup/tensor.h
	tests/test_djup.cpp
	tests/test_lexer.cpp
	tests/test_parse.cpp
	tests/test_pattern.cpp
	tests/test_tensor_to_string.cpp
)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)

target_link_libraries(djup core)