cmake_minimum_required(VERSION 3.5)

project(VEGAVideoServer LANGUAGES CXX)

IF(WIN32)
    SET(CMAKE_FIND_LIBRARY_PREFIXES "")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
ELSE()
    SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
ENDIF()


SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -fstrict-aliasing  -Wno-deprecated-declarations -Wno-unused-variable -Wno-volatile -Wno-address-of-packed-member")
SET(THREADS_PREFER_PTHREAD_FLAG ON)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


EXECUTE_PROCESS(COMMAND
    python mavlink/pymavlink/tools/mavgen.py --output c_library_v2 --lang C --wire-protocol 2.0 mavlink/message_definitions/v1.0/all.xml
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )

FIND_PATH(MAVLINK_INCLUDE_DIR
    NAMES
        protocol.h
    PATHS
    ${PROJECT_SOURCE_DIR}/c_library_v2)

find_package(Event REQUIRED)
find_package(Exiv REQUIRED)

IF (NOT MAVLINK_INCLUDE_DIR)
    message("Failed build mavlink")
ENDIF()
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/c_library_v2)
# end mavlink

SET(USE_RTSP_AUTHENTICATION False CACHE BOOL "")
# find ffmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
    libavdevice
    libavfilter
    libavformat
    libavcodec
    libswresample
    libswscale
    libavutil
)


add_executable(VEGAVideoServer
    helper.h
    MavContext.h MavContext.cpp
    MavlinkParser.h MavlinkParser.cpp
    FFImageFrame.h FFImageFrame.cpp
    FFDecoderInstance.h
    FFPlayerInstance.h FFPlayerInstance.cpp
    FFH264DecoderInstance.h FFH264DecoderInstance.cpp
    FFImageHttpSink.h FFImageHttpSink.cpp
    HTTPServer.h HTTPServer.cpp

    MavUDP.h MavUDP.cpp

    main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${LIBEVENT_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${LIBEXIV2_INCLUDE_DIR})


target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBEVENT_LIB})
target_link_libraries(${PROJECT_NAME} PUBLIC PkgConfig::LIBAV)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBEXIV2_LIB})

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
