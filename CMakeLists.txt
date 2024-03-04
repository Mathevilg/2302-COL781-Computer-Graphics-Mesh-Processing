cmake_minimum_required(VERSION 3.0)

project(a2)
set(CMAKE_CXX_STANDARD 11)

find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
add_library(viewer src/hw.cpp src/viewer.cpp deps/src/gl.c)
target_include_directories(viewer PUBLIC deps/include)
target_link_libraries(viewer glm::glm OpenGL::GL ${SDL2_LIBRARIES} -ldl)

add_executable(example src/example.cpp)
target_link_libraries(example viewer)
