cmake_minimum_required(VERSION 3.22)


project(askyart)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS NO)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

add_executable(askyart src/main.cpp)

target_compile_features(askyart PRIVATE cxx_std_11)
target_link_libraries(askyart ${OpenCV_LIBS})
target_link_libraries(askyart argparse)