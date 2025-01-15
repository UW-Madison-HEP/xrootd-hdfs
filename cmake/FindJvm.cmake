
FIND_PATH(JVM_INCLUDES jni.h
  HINTS
  ${JVM_INCLUDE_DIR}
  $ENV{JVM_INCLUDE_DIR}
  /usr/lib/jvm/java
  /usr/java/default
  /usr
  PATH_SUFFIXES include 
)

GET_FILENAME_COMPONENT(JVM_INCLUDE_PATH ${JVM_INCLUDES} PATH)

FIND_PATH(JVM_MD_INCLUDES jni_md.h
  HINTS
  ${JVM_INCLUDE_PATH}
  PATH_SUFFIXES include/linux
)

FIND_LIBRARY(JVM_LIB jvm
  HINTS
  ${JVM_LIB_DIR}
  $ENV{JVM_LIB_DIR}
  /usr
  /usr/java/default/jre
  /usr/lib/jvm/java/jre
  /usr/lib/jvm/jre
  PATH_SUFFIXES lib lib/amd64/server lib/i386/server lib/server
)

GET_FILENAME_COMPONENT(JVM_LIB_DIR ${JVM_LIB} PATH)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Jvm DEFAULT_MSG JVM_LIB JVM_INCLUDES JVM_MD_INCLUDES)

