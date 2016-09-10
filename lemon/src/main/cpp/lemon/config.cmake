include (CheckIncludeFiles)

CHECK_INCLUDE_FILES (sys/event.h LEMOON_KQUEUE_H)
CHECK_INCLUDE_FILES (sys/epoll.h LEMOON_HAS_EPOLL_H)

configure_file(${CMAKE_CURRENT_LIST_DIR}/config.h.in ${CMAKE_CURRENT_LIST_DIR}/config.h IMMEDIATE)
