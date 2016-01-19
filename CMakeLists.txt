
cmake_minimum_required( VERSION 2.6 )
project( xrootd-hdfs )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

find_package( Xrootd REQUIRED )
find_package( Hdfs REQUIRED )
find_package( Jvm REQUIRED )
find_package( Dl REQUIRED )

if( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )
endif()

if( CMAKE_COMPILER_IS_GNUCC )
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror" )
endif()

SET( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
SET( CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined")
SET( CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

include_directories("${HDFS_INCLUDES}" "${JVM_INCLUDES}" "${JVM_MD_INCLUDES}")

# Test for the correct connect API.
SET(ORIG_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
SET(CMAKE_REQUIRED_INCLUDES ${HDFS_INCLUDES} ${JVM_INCLUDES} ${JVM_MD_INCLUDES} ${CMAKE_REQUIRED_INCLUDES} )
SET(ORIG_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
SET(CMAKE_REQUIRED_LIBRARIES "-L${JVM_LIB_DIR} -ljvm -L${HDFS_LIB_DIR} -lhdfs ${CMAKE_REQUIRED_LIBRARIES}")

INCLUDE (CheckCSourceCompiles)
check_c_source_compiles("
#include \"hdfs.h\"
int main() {
    hdfsConnectAsUserNewInstance(\"default\", 0, \"nobody\");
    return 0;
}
" HAVE_NEW_HDFS)

SET(CMAKE_REQUIRED_LIBRARIES ${ORIG_CMAKE_REQUIRED_LIBRARIES})
SET(CMAKE_REQUIRED_INCLUDES ${ORIG_CMAKE_REQUIRED_INCLUDES})

if (HAVE_NEW_HDFS)
  add_definitions(-DHADOOP_VERSION=20)
else()
  add_definitions(-DHADOOP_VERSION=19)
endif()

include_directories( "${PROJECT_SOURCE_DIR}" "${XROOTD_INCLUDES}" )

add_library(XrdHdfs MODULE src/XrdHdfsBootstrap.cc)
target_link_libraries(XrdHdfs ${XROOTD_UTILS} ${DL_LIB})

add_library(XrdHdfsReal MODULE src/XrdHdfs.cc src/XrdHdfsConfig.cc src/XrdHdfs.hh)
target_link_libraries(XrdHdfsReal ${HDFS_LIB} ${XROOTD_UTILS})

add_executable(xrootd_hdfs_envcheck src/XrdHdfsEnvCheck.cc)

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  SET(CMAKE_INSTALL_LIBDIR "lib")
endif()

install(
  TARGETS XrdHdfs
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )

install(
  TARGETS XrdHdfsReal
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )

install(
  PROGRAMS xrootd_hdfs_envcheck
  DESTINATION bin)

install(
  FILES ${CMAKE_SOURCE_DIR}/src/XrdHdfs.hh
  DESTINATION include )

set_target_properties(
  XrdHdfs
  PROPERTIES
  VERSION   "0.0.1"
  SOVERSION "0"
  LINK_INTERFACE_LIBRARIES "" )

