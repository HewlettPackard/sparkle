# Find the xattr library.
# Output variables:
#  XATTR_INCLUDE_DIR : e.g., /usr/include/.
#  XATTR_LIBRARY     : Library path of xattr library
#  XATTR_FOUND       : True if found.
FIND_PATH(XATTR_INCLUDE_DIR NAME attr/xattr.h
  HINTS $ENV{HOME}/local/include /opt/local/include /usr/local/include /usr/include)

FIND_LIBRARY(XATTR_LIBRARY NAME attr
  HINTS $ENV{HOME}/local/lib64 $ENV{HOME}/local/lib /usr/local/lib64 /usr/local/lib /opt/local/lib64 /opt/local/lib /usr/lib64 /usr/lib
)

IF (XATTR_INCLUDE_DIR AND XATTR_LIBRARY)
    SET(XATTR_FOUND TRUE)
    MESSAGE(STATUS "Found xattr library: inc=${XATTR_INCLUDE_DIR}, lib=${XATTR_LIBRARY}")
ELSE ()
    SET(XATTR_FOUND FALSE)
    MESSAGE(STATUS "WARNING: xattr library not found.")
    MESSAGE(STATUS "Try: 'sudo apt-get install libxattr1 libxattr1-dev' (or sudo yum install libattr libattr-devel)")
ENDIF ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(XATTR REQUIRED_VARS XATTR_LIBRARY XATTR_INCLUDE_DIR)
