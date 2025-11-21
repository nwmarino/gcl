include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
)

add_compile_options(-g)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(gcl
    GCLContext.cpp
    Kernel.cpp
    ../vendor/spirv_reflect.cpp
)

target_include_directories(gcl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(gcl PRIVATE
    GTest::GTest
    GTest::Main
    Vulkan::Vulkan
)

target_compile_definitions(gcl PUBLIC SPIRV_REFLECT_USE_SYSTEM_SPIRV_H)

target_compile_features(gcl PUBLIC cxx_std_20)
