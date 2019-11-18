cmake_minimum_required(VERSION 3.14)
project(bashello)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
find_package(Readline)
find_package(Boost REQUIRED COMPONENTS filesystem  program_options regex)

add_executable(myshell src/main.cpp src/wildcards.cpp inc/wildcards.hpp src/builtins.cpp inc/builtins.hpp
        inc/parse_utils.hpp src/parse_utils.cpp src/custom_environ.cpp inc/custom_environ.hpp
        src/buf_containers.cpp inc/buf_containers.hpp src/command.cpp inc/command.hpp)
target_link_libraries(myshell ${Boost_LIBRARIES} ${Readline_LIBRARY})

include_directories(${Readline_INCLUDE_DIR})

#add_executable(mycat src/mycat.cpp)
#target_link_libraries(mycat ${Boost_LIBRARIES})
#
#add_executable(myls src/myls.cpp inc/myfile.hpp src/myfile.cpp)
#target_link_libraries(myls ${Boost_LIBRARIES})
