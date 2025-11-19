cmake_minimum_required(VERSION 4.0)

project(gcl VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED FALSE)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glm REQUIRED)
find_package(GTest REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(gcl SHARED
    source/GCLContext.cpp
    source/VulkanContext.cpp
)

add_library(gcl::gcl ALIAS gcl)

target_include_directories(gcl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(gcl PRIVATE
    glm::glm
    GTest::GTest
    GTest::Main
    vulkan
)

target_compile_features(gcl PUBLIC cxx_std_20)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
