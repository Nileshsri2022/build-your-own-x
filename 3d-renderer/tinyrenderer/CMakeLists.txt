cmake_minimum_required(VERSION 3.10)
project(tinyrenderer CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(tinyrenderer
    src/main.cpp
    src/tgaimage.cpp
    src/model.cpp
    src/renderer.cpp)
