cmake_minimum_required(VERSION 4.0)
project(gcl VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED FALSE)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(GTest REQUIRED)

add_subdirectory(source)

#if (GCL_COMPILE_EXAMPLES)
add_subdirectory(examples)
#endif()
