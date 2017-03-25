
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

include_directories( "${PROJECT_SOURCE_DIR}" "${XROOTD_INCLUDES}" )

add_library(XrdHdfs MODULE src/XrdHdfsBootstrap.cc)
target_link_libraries(XrdHdfs ${XROOTD_UTILS} ${DL_LIB})

add_library(XrdHdfsReal MODULE src/XrdHdfs.cc src/XrdHdfsConfig.cc src/XrdHdfs.hh)
target_link_libraries(XrdHdfsReal ${HDFS_LIB} ${XROOTD_UTILS})

add_executable(xrootd_hdfs_envcheck src/XrdHdfsEnvCheck.cc)

if (NOT DEFINED LIB_INSTALL_DIR)
  SET(LIB_INSTALL_DIR "lib")
endif()

install(
  TARGETS XrdHdfs
  LIBRARY DESTINATION ${LIB_INSTALL_DIR} )

install(
  TARGETS XrdHdfsReal
  LIBRARY DESTINATION ${LIB_INSTALL_DIR} )

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

