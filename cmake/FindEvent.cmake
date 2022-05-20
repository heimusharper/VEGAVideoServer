find_path(LIBEVENT_INCLUDE_DIR
    event.h
  PATHS
    /usr/include
    /usr/local/include
    /opt/include
  PATH_SUFFIXES
    include
)

IF(WIN32)
    FIND_LIBRARY(LIBEVENT_LIBRARY_DIR
        NAMES
            event
        PATHS
            /bin
        PATH_SUFFIXES lib
    )
ELSE()
    FIND_LIBRARY(LIBEVENT_LIBRARY_DIR
        NAMES
            event
        PATHS
            /lib
            /usr/lib/x86_64-linux-gnu
            /usr/local/lib64
            /usr/local/lib
            /usr/lib
        PATH_SUFFIXES
            lib
            lib64
    )
ENDIF()

SET(LIBEVENT_INCLUDE_DIRS ${LIBEVENT_INCLUDE_DIR})
SET(LIBEVENT_LIBRARY_DIRS ${LIBEVENT_LIBRARY_DIR})

SET(LIBEVENT_FOUND "NO")
IF (LIBEVENT_INCLUDE_DIRS AND LIBEVENT_LIBRARY_DIRS)
    SET(LIBEVENT_FOUND "YES")
ENDIF()
