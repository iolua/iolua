
if(WIN32)
    add_definitions(/DLEMOON_BUILD /wd4244 /D_UNICODE)
elseif(APPLE)
    add_definitions(-D_BSD_SOURCE)
else()
    add_definitions(-D_POSIX_SOURCE -DLUA_USE_LINUX)
endif()


file(GLOB_RECURSE src_files
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)

file(GLOB_RECURSE header_files
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx)

add_library(libiolua STATIC ${header_files} ${src_files})


target_link_libraries(libiolua lemon)

foreach(FILE ${header_files})
    get_filename_component(FILE_NAME ${FILE} NAME)
    string(REPLACE ${FILE_NAME} "" DIRECTORY ${FILE})

    file(RELATIVE_PATH DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} ${DIRECTORY})

    file(TO_NATIVE_PATH "${DIRECTORY}" DIRECTORY)

    source_group("include\\${DIRECTORY}" FILES ${FILE})
endforeach()

foreach(FILE ${src_files})
    get_filename_component(FILE_NAME ${FILE} NAME)
    string(REPLACE ${FILE_NAME} "" DIRECTORY ${FILE})

    file(RELATIVE_PATH DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} ${DIRECTORY})

    file(TO_NATIVE_PATH "${DIRECTORY}" DIRECTORY)

    source_group("sources\\${DIRECTORY}" FILES ${FILE})
endforeach()
