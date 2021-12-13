cmake_minimum_required(VERSION 3.16)
project(acm-fakestore)

set(CMAKE_CXX_STANDARD 20)

set(LIBAKCPP_DIR ${acm-fakestore_SOURCE_DIR}/vendor/libakcpp)

execute_process(COMMAND git submodule update --recursive)
execute_process(COMMAND cmake . WORKING_DIRECTORY ${LIBAKCPP_DIR})
execute_process(COMMAND make WORKING_DIRECTORY ${LIBAKCPP_DIR})

add_executable(code src/dummy.cpp)
target_include_directories(code PRIVATE ${LIBAKCPP_DIR}/include)
target_link_libraries(code ${LIBAKCPP_DIR}/libakcpp.a)
