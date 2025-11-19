include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_library(gcl
    Buffer.cpp
    GCLContext.cpp
    VulkanContext.cpp
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

target_compile_features(gcl PUBLIC cxx_std_20)
