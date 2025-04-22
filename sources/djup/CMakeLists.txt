cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4)
endif()

add_library(djup STATIC
	public/djup/expression.h
	public/djup/name.h
	public/djup/tensor.h
	private/pattern/candidate.h
	private/pattern/candidate.cpp
	private/pattern/discrimination_tree.h
	private/pattern/discrimination_tree.cpp
	private/pattern/pattern_info.h
	private/pattern/pattern_info.cpp
	private/pattern/substitution_graph.h
	private/pattern/substitution_graph.cpp
	private/pattern/substitution_graph_to_dot_language.cpp
	private/pattern/debug_utils.h
	private/pattern/debug_utils.cpp
	private/common.h
	private/alphabet.h
	private/builtin_names.h
	private/constant_shape.h
	private/../tests/test_utils.h
	private/../tests/test_utils.cpp
	private/indices.h
	private/lexer.h	
	private/namespace.h
	private/parser.h
	private/standard_scope.h
	private/substitute_by_predicate.h
	private/tensor_map.h
	private/constant_shape.cpp
	private/expression.cpp
	private/tensor_to_string.cpp
	private/indices.cpp
	private/is.cpp
	private/lexer.cpp
	private/name.cpp
	private/namespace.cpp
	private/parser.cpp
	private/old_pattern_match.h
	private/old_pattern_match.cpp
	private/standard_namespace.cpp
	private/tensor_to_graph.cpp
	private/tensor.cpp
	tests/test_djup.cpp
	tests/test_lexer.cpp
	tests/test_parse.cpp
	tests/test_old_pattern.cpp
	tests/test_tensor_to_graph.cpp
	tests/test_pattern.cpp
	tests/test_pattern_info.cpp
	tests/test_tensor_to_string.cpp
)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)

target_link_libraries(djup core)