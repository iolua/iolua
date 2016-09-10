cmake_minimum_required(VERSION 3.2)
project(iolua)

set(GSMAKE_VERSION "3.0.*")

# open user folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
    foreach(flag_var CMAKE_C_FLAGS CMAKE_CXX_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
        string(REGEX REPLACE "/W3" "/W4" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g3")
    message(${CMAKE_CXX_FLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
endif()

include_directories(${CMAKE_SOURCE_DIR}/lemon/src/main/cpp)
include_directories(${CMAKE_SOURCE_DIR}/iolua/src/main/cpp)

add_subdirectory(lemon)
add_subdirectory(iolua)
