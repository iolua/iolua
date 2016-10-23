include (CheckIncludeFiles)

CHECK_INCLUDE_FILES (sys/event.h LEMOON_KQUEUE_H)
CHECK_INCLUDE_FILES (sys/epoll.h LEMOON_HAS_EPOLL_H)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lemon/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/lemon/config.h IMMEDIATE)

if(MSVC)
    add_definitions(/DLEMOON_BUILD /wd4311 /wd4310	 /D_UNICODE)
elseif(APPLE)
    add_definitions(-D_BSD_SOURCE)
elseif(MINGW)
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

add_library(lemon STATIC ${header_files} ${src_files})


if(WIN32)
elseif(APPLE)
else()
    target_link_libraries(lemon pthread dl)
endif()



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
