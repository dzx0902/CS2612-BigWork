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
  src/bison_adapter.c
)
target_include_directories(fol PRIVATE . src)

# Optional Bison integration
find_program(BISON_EXECUTABLE NAMES win_bison bison)
if (BISON_EXECUTABLE)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/parser.c ${CMAKE_BINARY_DIR}/parser.h
    COMMAND ${BISON_EXECUTABLE} -d -o ${CMAKE_BINARY_DIR}/parser.c ${CMAKE_SOURCE_DIR}/src/parser.y
    DEPENDS ${CMAKE_SOURCE_DIR}/src/parser.y
    COMMENT "Generating Bison parser"
  )
  add_custom_target(bison_gen DEPENDS ${CMAKE_BINARY_DIR}/parser.c ${CMAKE_BINARY_DIR}/parser.h)
  add_dependencies(fol bison_gen)
  target_sources(fol PRIVATE ${CMAKE_BINARY_DIR}/parser.c)
  target_include_directories(fol PRIVATE ${CMAKE_BINARY_DIR})
  target_compile_definitions(fol PRIVATE HAVE_BISON_PARSER=1)
endif()

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

add_executable(test_errors
  src/test_errors.c
  src/lexer.c
  src/ast.c
  src/rdparser.c
)
target_include_directories(test_errors PRIVATE . src)

include(CTest)
if (BUILD_TESTING)
  add_test(NAME test_ast COMMAND $<TARGET_FILE:test_ast>)
  add_test(NAME test_parser COMMAND $<TARGET_FILE:test_parser>)
  add_test(NAME test_transform COMMAND $<TARGET_FILE:test_transform>)
  add_test(NAME test_polarity COMMAND $<TARGET_FILE:test_polarity>)
  add_test(NAME test_errors COMMAND $<TARGET_FILE:test_errors>)
endif()
