
cmake_minimum_required( VERSION 2.6 )
project( xrootd-hdfs )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

find_package( Xrootd REQUIRED )
find_package( Hdfs REQUIRED )
find_package( Jvm REQUIRED )
find_package( Dl REQUIRED )

include (FindPkgConfig)
pkg_check_modules(LIBCRYPTO REQUIRED libcrypto)
pkg_check_modules(ZLIB REQUIRED zlib)

macro(use_cxx11)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 11)
  endif ()
endmacro(use_cxx11)
use_cxx11()

if( CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror" )
endif()

if( CMAKE_COMPILER_IS_GNUCC )
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror" )
endif()

SET( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
SET( CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined")
SET( CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined")

include_directories(${HDFS_INCLUDES} ${JVM_INCLUDES} ${JVM_MD_INCLUDES} ${LIBCRYPTO_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})

include_directories( "${PROJECT_SOURCE_DIR}" "${XROOTD_INCLUDES}" )

add_library(XrdHdfs MODULE src/XrdHdfsBootstrap.cc src/XrdHdfsChecksum.cc src/XrdHdfsChecksumCalc.cc)
target_link_libraries(XrdHdfs ${XROOTD_UTILS} ${DL_LIB} ${LIBCRYPTO_LIBRARIES} ${ZLIB_LIBRARIES})
set_target_properties(XrdHdfs PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/src/export-lib-symbols")

add_library(XrdHdfsReal MODULE src/XrdHdfs.cc src/XrdHdfsConfig.cc src/XrdHdfs.hh src/XrdHdfsChecksum.cc src/XrdHdfsChecksumCalc.cc)
target_link_libraries(XrdHdfsReal ${HDFS_LIB} ${XROOTD_UTILS} ${LIBCRYPTO_LIBRARIES} ${ZLIB_LIBRARIES})
set_target_properties(XrdHdfsReal PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_SOURCE_DIR}/src/export-lib-symbols")

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
  TARGETS xrootd_hdfs_envcheck
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

