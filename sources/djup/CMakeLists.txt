cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

add_library(djup STATIC
	private/pattern/discrimination_tree.h
	private/pattern/pattern_info.h
	private/pattern/substitution_graph.h
	private/pattern/discrimination_tree.cpp
	private/pattern/pattern_info.cpp
	private/pattern/substitution_graph.cpp
	private/pattern/candidate.cpp
	private/pattern/utils.h
	private/pattern/utils.cpp
	private/pattern/substitution_graph_to_dot_language.cpp
	private/pattern/substitution_match.cpp
	private/alphabet.h
	private/builtin_names.h
	private/constant_shape.h
	private/diagnostic.h
	private/expression.h
	private/indices.h
	private/lexer.h
	private/name.h
	private/namespace.h
	private/parser.h
	private/old_pattern_match.h
	private/standard_scope.h
	private/substitute_by_predicate.h
	private/tensor_map.h
	private/constant_shape.cpp
	private/expression.cpp
	private/expression_to_string.cpp
	private/indices.cpp
	private/is.cpp
	private/lexer.cpp
	private/name.cpp
	private/namespace.cpp
	private/parser.cpp
	private/old_pattern_match.cpp
	private/standard_namespace.cpp
	private/tensor.cpp
	public/djup/tensor.h
	tests/test_djup.cpp
	tests/test_lexer.cpp
	tests/test_parse.cpp
	tests/test_old_pattern.cpp
	tests/test_pattern.cpp
	tests/test_tensor_to_string.cpp
)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)

target_link_libraries(djup core)