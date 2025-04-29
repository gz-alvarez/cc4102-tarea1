cmake_minimum_required(VERSION 3.10.0)
project(logs-t1 VERSION 0.1.0 LANGUAGES C CXX)

#set(COMPILE_FLAGS "${COMPILE_FLAGS} --std=c++23")
set(CMAKE_CXX_STANDARD 23)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(logs-t1 ${SRC_FILES})
target_include_directories(logs-t1 PRIVATE ${PROJECT_SOURCE_DIR}/include)