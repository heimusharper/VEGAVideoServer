find_path(LIBEXIV2_INCLUDE_DIR
    exiv2/exiv2.hpp
  PATHS
    /usr/include
    /usr/local/include
    /opt/include
  PATH_SUFFIXES
    include
)

IF(WIN32)
    FIND_LIBRARY(LIBEXIV2_LIBRARY_DIR
        NAMES
            exiv2
        PATHS
            /bin
        PATH_SUFFIXES lib
    )
ELSE()
    FIND_LIBRARY(LIBEXIV2_LIBRARY_DIR
        NAMES
            exiv2
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

SET(LIBEXIV2_INCLUDE_DIRS ${LIBEXIV2_INCLUDE_DIR})
SET(LIBEXIV2_LIBRARY_DIRS ${LIBEXIV2_LIBRARY_DIR})

SET(LIBEXIV2_FOUND "NO")
IF (LIBEXIV2_INCLUDE_DIRS AND LIBEXIV2_LIBRARY_DIRS)
    SET(LIBEXIV2_FOUND "YES")
ENDIF()
