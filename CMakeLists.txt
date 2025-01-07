cmake_minimum_required(VERSION 3.20)
project(WebServer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


add_subdirectory(tests)
add_subdirectory(library/base)
add_subdirectory(library/net)
add_subdirectory(library/thread)