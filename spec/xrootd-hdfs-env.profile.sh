#!/bin/sh

XROOTD_USER=`source /etc/sysconfig/xrootd; echo $XROOTD_USER`

if [ "x$XROOTD_USER" = "x$USER" ]; then
  source /etc/sysconfig/xrootd-hdfs
fi

