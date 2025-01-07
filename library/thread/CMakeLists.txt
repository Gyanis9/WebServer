add_library(Thread_Pool SHARED
        src/Thread.cpp
        src/ThreadPool.cpp
        include/Thread.h
        include/ThreadPool.hpp
        src/CurrentThread.cpp
        include/CurrentThread.h
)
#find_package(benchmark REQUIRED)
#
#target_include_directories(Thread_Pool
#        PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#)