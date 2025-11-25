cmake_minimum_required(VERSION 3.15)
project(fol LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(fol
  src/main.c
  src/lexer.c
  src/ast.c
  src/rdparser.c
  src/transform.c
  src/polarity.c
  src/print.c
  src/sb.c
)
target_include_directories(fol PRIVATE . src)

add_executable(test_ast
  src/test_ast.c
  src/ast.c
)
target_include_directories(test_ast PRIVATE . src)

add_executable(test_parser
  src/test_parser.c
  src/lexer.c
  src/ast.c
  src/rdparser.c
)
target_include_directories(test_parser PRIVATE . src)

add_executable(test_transform
  src/test_transform.c
  src/lexer.c
  src/ast.c
  src/rdparser.c
  src/transform.c
)
target_include_directories(test_transform PRIVATE . src)

add_executable(test_polarity
  src/test_polarity.c
  src/lexer.c
  src/ast.c
  src/rdparser.c
  src/polarity.c
)
target_include_directories(test_polarity PRIVATE . src)

include(CTest)
if (BUILD_TESTING)
  add_test(NAME test_ast COMMAND $<TARGET_FILE:test_ast>)
  add_test(NAME test_parser COMMAND $<TARGET_FILE:test_parser>)
  add_test(NAME test_transform COMMAND $<TARGET_FILE:test_transform>)
endif()
