set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(examples
    main.cpp
)

target_link_libraries(examples PRIVATE gcl)

target_compile_features(examples PRIVATE cxx_std_20)
