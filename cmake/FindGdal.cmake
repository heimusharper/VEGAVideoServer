FIND_PATH(Gdal_INCLUDE_DIR
    NAMES 
        gdal.h
    PATHS 
        /usr/local/include/gdal
        /usr/local/include
        /usr/include/gdal
        /usr/include
        /include/gdal
        /include
)
IF(WIN32)
    FIND_LIBRARY(Gdal_LIBRARY_DIR
        NAMES
            gdal gdal-28
        PATHS
            /bin
        PATH_SUFFIXES lib
    )
ELSE()
    FIND_LIBRARY(Gdal_LIBRARY_DIR
        NAMES
            gdal
        PATHS
            /lib
            /usr/local/lib64
            /usr/local/lib
            /usr/lib
        PATH_SUFFIXES lib
    )
ENDIF()

SET(Gdal_INCLUDE_DIRS ${Gdal_INCLUDE_DIR})
SET(Gdal_LIBRARY_DIRS ${Gdal_LIBRARY_DIR})

SET(Gdal_FOUND "NO")
IF (Gdal_INCLUDE_DIRS AND Gdal_LIBRARY_DIRS)
    SET(Gdal_FOUND "YES")
ENDIF()
