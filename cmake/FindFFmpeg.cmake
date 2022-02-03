find_path(FFMPEG_INCLUDE_DIR 
  libavcodec/avcodec.h
  PATHS
    /home/unmanned/ffmpeg-4.2.2/install
    /usr
    /usr/local
  PATH_SUFFIXES
    include
)

find_library(NVMPI_LIBRARY nvmpi)
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVUTIL_LIBRARY avutil)
find_library(AVDEVICE_LIBRARY avdevice)
find_library(SWSCALE_LIBRARY swscale)
find_library(SWRESAMPLE_LIBRARY swresample)

SET(AV_LIBS
    ${NVMPI_LIBRARY}
    ${AVCODEC_LIBRARY}
    ${AVFORMAT_LIBRARY}
    ${AVUTIL_LIBRARY}
    ${AVDEVICE_LIBRARY}
    ${SWSCALE_LIBRARY}
    ${SWRESAMPLE_LIBRARY}
    )
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
message("find in ${FFMPEG_LIB} ${FFMPEG_INCLUDE_DIR}")
