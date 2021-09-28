cmake_minimum_required(VERSION 3.0.0)
project(QuadtreeAmoguifier VERSION 0.1.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

add_executable(QuadtreeAmoguifier Image.cpp main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_command(TARGET QuadtreeAmoguifier
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:QuadtreeAmoguifier> ${PROJECT_SOURCE_DIR})