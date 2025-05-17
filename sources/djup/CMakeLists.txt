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

    #headers
    private/alphabet.h
    private/builtin_names.h
    private/common.h
    private/constant_shape.h
    private/expression.h
    private/indices.h
    private/int_interval.h
    private/lexer.h
    private/m2o_pattern/m2o_candidate.h
    private/m2o_pattern/m2o_debug_utils.h
    private/m2o_pattern/m2o_discrimination_tree.h
    private/m2o_pattern/m2o_pattern_info.h
    private/m2o_pattern/m2o_substitution_graph.h
    private/m2o_pattern/m2o_substitutions_builder.h
    private/make_expr.h
    private/namespace.h
    private/o2o_pattern/o2o_debug_utils.h
    private/o2o_pattern/o2o_pattern_info.h
    private/o2o_pattern/o2o_pattern_match.h
    private/o2o_pattern/o2o_substitutions_builder.h
    private/old_pattern_match.h
    private/parser.h
    private/standard_scope.h
    private/substitute_by_predicate.h
    private/tensor_type.h
    public/djup/tensor.h
    tests/test_utils.h

    #cpps
    private/constant_shape.cpp
    private/expression.cpp
    private/indices.cpp
    private/int_interval.cpp
    private/is.cpp
    private/lexer.cpp
    private/m2o_pattern/m2o_candidate.cpp
    private/m2o_pattern/m2o_debug_utils.cpp
    private/m2o_pattern/m2o_discrimination_tree.cpp
    private/m2o_pattern/m2o_pattern_info.cpp
    private/m2o_pattern/m2o_substitution_graph.cpp
    private/m2o_pattern/m2o_substitution_graph_to_dot_language.cpp
    private/m2o_pattern/m2o_substitution_match.cpp
    private/m2o_pattern/m2o_substitutions_builder.cpp
    private/make_expr.cpp
    private/namespace.cpp
    private/o2o_pattern/o2o_apply_substitutions.cpp
    private/o2o_pattern/o2o_debug_utils.cpp
    private/o2o_pattern/o2o_pattern_info.cpp
    private/o2o_pattern/o2o_pattern_match.cpp
    private/o2o_pattern/o2o_substitutions_builder.cpp
    private/old_pattern_match.cpp
    private/parser.cpp
    private/standard_namespace.cpp
    private/tensor.cpp
    private/tensor_to_graph.cpp
    private/tensor_to_string.cpp
    private/tensor_type.cpp
    tests/test_djup.cpp
    tests/test_lexer.cpp
    tests/test_m2o_discrimination_tree.cpp
    tests/test_m2o_pattern.cpp
    tests/test_m2o_pattern_info.cpp
    tests/test_m2o_substitutions_builder.cpp
    tests/test_o2o_pattern.cpp
    tests/test_old_pattern.cpp
    tests/test_parse.cpp
    tests/test_tensor_to_graph.cpp
    tests/test_tensor_to_string.cpp
    tests/test_tensor_type.cpp
    tests/test_utils.cpp
)

target_include_directories(djup PUBLIC public)
target_include_directories(djup PRIVATE .)
target_include_directories(djup PRIVATE .)

target_link_libraries(djup core)