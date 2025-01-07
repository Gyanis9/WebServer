add_executable(WebServer WebServer.cpp)
#set(CMAKE_PREFIX_PATH "/usr/local/muduo/lib/cmake/muduo" CACHE PATH "Path to Muduo cmake config")
#
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
#find_package(muduo REQUIRED)
target_link_libraries(WebServer PRIVATE  net)