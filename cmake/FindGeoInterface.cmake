FIND_PATH(GeoInterface_INCLUDE_DIR
    NAMES 
        HeightSource.h
    PATHS 
        /usr/local/include/gdal
        /usr/local/include
        /usr/include/gdal
        /usr/include
        /include/gdal
        /include
)
IF(WIN32)
    FIND_LIBRARY(GeoInterface_LIBRARY_DIR
        NAMES
            GeoInterface
        PATHS
            /bin
        PATH_SUFFIXES lib
    )
ELSE()
    FIND_LIBRARY(GeoInterface_LIBRARY_DIR
        NAMES
            GeoInterface
        PATHS
            /lib
            /usr/local/lib64
            /usr/local/lib
            /usr/lib
        PATH_SUFFIXES lib
    )
ENDIF()

SET(GeoInterface_INCLUDE_DIRS ${GeoInterface_INCLUDE_DIR})
SET(GeoInterface_LIBRARY_DIRS ${GeoInterface_LIBRARY_DIR})

SET(GeoInterface_FOUND "NO")
IF (GeoInterface_INCLUDE_DIRS AND GeoInterface_LIBRARY_DIRS)
    SET(GeoInterface_FOUND "YES")
ENDIF()
