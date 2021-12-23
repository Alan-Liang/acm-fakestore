cmake_minimum_required(VERSION 3.16)
project(acm-fakestore)

set(CMAKE_CXX_STANDARD 20)

set(LIBAKCPP_DIR ${acm-fakestore_SOURCE_DIR}/vendor/libakcpp)

execute_process(COMMAND git submodule init)
execute_process(COMMAND git submodule update --recursive)
execute_process(COMMAND cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON . WORKING_DIRECTORY ${LIBAKCPP_DIR})
execute_process(COMMAND make WORKING_DIRECTORY ${LIBAKCPP_DIR})

set(SOURCES
  src/main.cpp
  src/books.cpp
  src/users.cpp
)

add_executable(code ${SOURCES})
target_include_directories(code PRIVATE ${LIBAKCPP_DIR}/include)
target_link_libraries(code ${LIBAKCPP_DIR}/libakcpp.a)
