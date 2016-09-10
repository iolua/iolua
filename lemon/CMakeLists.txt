if(WIN32)
    add_definitions(/DLEMOON_BUILD /wd4311 /D_UNICODE)
elseif(APPLE)
    add_definitions(-D_BSD_SOURCE)
else()
    add_definitions(-D_POSIX_SOURCE)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp)

add_subdirectory(src/main/cpp)
add_subdirectory(src/test/cpp)
