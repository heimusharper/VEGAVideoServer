find_path(FFMPEG_INCLUDE_DIRS
    libavcodec/avcodec.h
  PATHS
    /home/unmanned/ffmpeg-4.2.2/install
    /usr/local
    /usr
  PATH_SUFFIXES
    include
)


FIND_LIBRARY(NVMPI_LIBRARY
    NAMES
        nvmpi
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

FIND_LIBRARY(AVCODEC_LIBRARY
    NAMES
        avcodec
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

FIND_LIBRARY(AVFORMAT_LIBRARY
    NAMES
        avformat
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

FIND_LIBRARY(AVUTIL_LIBRARY
    NAMES
        avutil
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

FIND_LIBRARY(AVDEVICE_LIBRARY
    NAMES
        avdevice
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

FIND_LIBRARY(SWSCALE_LIBRARY
    NAMES
        swscale
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

FIND_LIBRARY(SWRESAMPLE_LIBRARY
    NAMES
        swresample
    PATHS
        /lib
        /usr/local/lib64
        /usr/local/lib
        /usr/lib
    PATH_SUFFIXES lib
)

SET(AV_LIBS
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${AVDEVICE_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${SWRESAMPLE_LIBRARY}
    )
SET(USE_NVMPI False)
IF (NVMPI_LIBRARY)
    SET(AV_LIBS
        ${AV_LIBS}
        ${NVMPI_LIBRARY}
        )
    SET(USE_NVMPI True)
ENDIF()

#find_library(Z_LIB
#  NAMES
#    z
#  PATHS
#    /usr/local
#    /opt
#    /usr/lib/aarch64-linux-gnu
#  PATH_SUFFIXES
#    lib
#    lib64
#)

#set(FFMPEG_INCLUDE_DIR "/home/unmanned/ffmpeg-4.2.2/install/include")
#set(FFMPEG_LIB ${Z_LIB})
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libavdevice.a")
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libavfilter.a")
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libavformat.a")
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libavcodec.a")
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libswresample.a")
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libswscale.a")
#set(FFMPEG_LIB ${FFMPEG_LIB} "/home/unmanned/ffmpeg-4.2.2/install/lib/libavutil.a")
message("find in ${AV_LIBS} ${FFMPEG_INCLUDE_DIR}")
