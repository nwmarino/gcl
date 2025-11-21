set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(EXAMPLE_SOURCES
    branch.cpp
    heavy.cpp
    ma.cpp
)

foreach(src IN LISTS EXAMPLE_SOURCES)
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE gcl)
    target_compile_features(${name} PRIVATE cxx_std_20)
endforeach()
