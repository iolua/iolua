if(MSVC)
    add_definitions(/DLEMOON_BUILD /wd4311 /D_UNICODE)
elseif(APPLE)
    add_definitions(-D_BSD_SOURCE)
else()
    add_definitions(-D_POSIX_SOURCE)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_subdirectory(src/main/cpp)
add_subdirectory(src/test/cpp)

add_executable(iolua main.cpp )

target_link_libraries(iolua lemon libiolua)

add_dependencies(iolua lemon libiolua)

set_target_properties(
        iolua
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/)