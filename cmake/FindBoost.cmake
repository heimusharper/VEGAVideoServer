FIND_PATH(Boost_INCLUDE_DIR
    NAMES
        circular_buffer.hpp
    PATHS
        /usr/local/include/boost
        /usr/local/include
        /usr/include/boost
        /usr/include
        )

MACRO ( FIND_Boost_LIBRARY MYLIBRARY MYLIBRARYNAME )
    FIND_LIBRARY(${MYLIBRARY}
        NAMES
            ${MYLIBRARYNAME}
        PATHS
            /usr/lib/x86_64-linux-gnu
            /usr/local/lib64
            /usr/local/lib
            /usr/lib
            /lib)
ENDMACRO()

FIND_Boost_LIBRARY(BOOST_CONTAINER_LIBRARY boost_container)
SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${BOOST_CONTAINER_LIBRARY} )

FIND_Boost_LIBRARY(BOOST_PROGRAM_OPTIONS_LIBRARY boost_program_options)
SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${BOOST_PROGRAM_OPTIONS_LIBRARY} )

FIND_Boost_LIBRARY(BOOST_LOG_SETUP_LIBRARY boost_log_setup)
SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${BOOST_LOG_SETUP_LIBRARY} )

FIND_Boost_LIBRARY(BOOST_THREAD_LIBRARY boost_thread)
SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${BOOST_THREAD_LIBRARY} )

FIND_Boost_LIBRARY(BOOST_SYSTEM_LIBRARY boost_system)
SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${BOOST_SYSTEM_LIBRARY} )

FIND_Boost_LIBRARY(BOOST_CHRONO_LIBRARY boost_chrono)
SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${BOOST_CHRONO_LIBRARY} )

SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})

SET(Boost_FOUND "NO")
IF (Boost_INCLUDE_DIRS AND Boost_LIBRARY_DIRS)
    SET(Boost_FOUND "YES")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -DBOOST_LOG_DYN_LINK")
ENDIF()
