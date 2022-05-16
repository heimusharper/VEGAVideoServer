FIND_PATH(Spdlog_INCLUDE_DIR
    NAMES
        spdlog/common.h
    PATHS
        /usr/local/include
        /usr/include
)
FIND_PATH(Fmt_INCLUDE_DIR
    NAMES
        fmt/format.h
    PATHS
        /usr/local/include
        /usr/include
)

FIND_LIBRARY(Fmt_LIBRARY_DIR
    NAMES
        fmt
    PATHS
        /usr/lib/x86_64-linux-gnu
        /usr/lib
)

SET(Spdlog_INCLUDE_DIRS ${Spdlog_INCLUDE_DIR} ${Fmt_INCLUDE_DIR})
SET(Fmt_LIBRARY_DIRS ${Fmt_LIBRARY_DIR})

message("SPD ${Fmt_LIBRARY_DIRS} ${Spdlog_INCLUDE_DIRS}")

SET(Spdlog_FOUND "NO")
IF (Spdlog_INCLUDE_DIRS)
    SET(Spdlog_FOUND "YES")
ENDIF()
