cmake_minimum_required(VERSION 3.29.0)
project(jcl2jcl VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

FILE(GLOB MyCSources src/*.c)
add_executable(jcl2jcl ${MyCSources})
target_include_directories(jcl2jcl PRIVATE src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
