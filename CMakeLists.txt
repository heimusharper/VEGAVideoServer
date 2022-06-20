cmake_minimum_required(VERSION 3.10)

project(VEGAVideoServer LANGUAGES CXX)

IF(WIN32)
    SET(CMAKE_FIND_LIBRARY_PREFIXES "")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
ELSE()
    SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
ENDIF()


# ASAN CONFIGURE
SET(ASAN_ADDRESS True CACHE BOOL "Compile address sanitizer")
SET(ASAN_THREADS False CACHE BOOL "Compile thread sanitizer")
SET(ASAN_UB True CACHE BOOL "Compile undefined behaviour sanitizer")

IF (${ASAN_ADDRESS})
    #SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -g")
    #SET(CMAKE_CXX_LINKER_FLAGS_DEBUG "${CMAKE_CXX_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak")

    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -shared-libasan -g ")
        set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    ELSE()
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -g ")
        set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    ENDIF()

ENDIF ()
IF(${ASAN_THREADS})
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread -g")
    SET(CMAKE_CXX_LINKER_FLAGS_DEBUG "${CMAKE_CXX_LINKER_FLAGS_DEBUG} -fsanitize=thread")
ENDIF()
IF (${ASAN_UB})
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -g")
    SET(CMAKE_CXX_LINKER_FLAGS_DEBUG "${CMAKE_CXX_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
ENDIF()

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lz")
SET(THREADS_PREFER_PTHREAD_FLAG ON)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


EXECUTE_PROCESS(COMMAND
    python3 mavlink/pymavlink/tools/mavgen.py --output c_library_v2 --lang C --wire-protocol 2.0 mavlink/message_definitions/v1.0/all.xml
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

FIND_PATH(MAVLINK_INCLUDE_DIR
    NAMES
        protocol.h
    PATHS
    ${PROJECT_SOURCE_DIR}/c_library_v2)

FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(FFmpeg REQUIRED)
FIND_PACKAGE(Event REQUIRED)
FIND_PACKAGE(Exiv REQUIRED)
FIND_PACKAGE(Gdal REQUIRED)
FIND_PACKAGE(GeoInterface REQUIRED)
FIND_PACKAGE(Spdlog)

IF (NOT MAVLINK_INCLUDE_DIR)
    message("Failed build mavlink")
ENDIF()
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/c_library_v2)
# end mavlink

SET(USE_RTSP_AUTHENTICATION False CACHE BOOL "")

IF (${USE_NVMPI})
	#    add_definitions(-DUSE_NVMPI)
ENDIF()

INCLUDE_DIRECTORIES(${Spdlog_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Gdal_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GeoInterface_INCLUDE_DIRS})

SET(HEADERS_LIST
    log.h
    helper.h
    Decoder.h
    MavContext.h
    IPacketReader.h
    VideoSource.h

    filesink/FileSave.h

    httpsink/EncoderJPEG.h
    httpsink/JPEGHttpSink.h
    httpsink/HTTPServer.h

    telem/MavlinkParser.h
    telem/MavUDP.h

    )
SET(SOURCES_LIST
    Decoder.cpp
    MavContext.cpp
    VideoSource.cpp

    filesink/FileSave.cpp

    httpsink/EncoderJPEG.cpp
    httpsink/JPEGHttpSink.cpp
    httpsink/HTTPServer.cpp

    telem/MavlinkParser.cpp
    telem/MavUDP.cpp

    main.cpp
    )

ADD_EXECUTABLE(VEGAVideoServer ${HEADERS_LIST} ${SOURCES_LIST})

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${LIBEVENT_INCLUDE_DIRS})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE ${LIBEXIV2_INCLUDE_DIRS})


TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${Boost_LIBRARY_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${GeoInterface_LIBRARY_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${LIBEXIV2_LIBRARY_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${AV_LIBS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${LIBEVENT_LIBRARY_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${Gdal_LIBRARY_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${Fmt_LIBRARY_DIRS})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
# install(FILES ${HEADERS_LIST} DESTINATION include)
