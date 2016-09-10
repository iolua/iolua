file(GLOB_RECURSE src_files *.c *.cpp *.cxx *.h *.hpp)

add_executable(lemon-test ${src_files})

target_link_libraries(lemon-test lemon)

set_target_properties(
        lemon-test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/)