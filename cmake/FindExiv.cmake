find_path(LIBEXIV2_INCLUDE_DIR exiv2/exiv2.hpp
  PATHS
    /usr
    /usr/local
    /opt
  PATH_SUFFIXES
    include
)

find_library(LIBEXIV2_LIB
  NAMES
    exiv2
  PATHS
    /usr/local
    /opt
  PATH_SUFFIXES
    lib
    lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  LIBEXIV2_LIB
  LIBEXIV2_INCLUDE_DIR
)
