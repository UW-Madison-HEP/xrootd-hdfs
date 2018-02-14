# A HDFS filesystem plugin for XRootD

This plugin allows an XRootD host to talk directly to a HDFS filesystem
via the OSS plugin layer.

To enable, add the following line to your Xrootd build:

```
ofs.osslib /usr/lib64/libXrdHdfs.so
```

Once enabled, it will use the default locations to bootstrap the HDFS configuration
(currently `/etc/hadoop/conf`); if the output of `hdfs dfs -ls` shows your Hadoop instance,
not the local POSIX filesystem, you should be set.

One complex part of launching an embedded JVM is determining the `CLASSPATH` variable.  To
do this, the plugin will internally perform the following:

```
source /etc/sysconfig/xrootd-hdfs && /usr/libexec/xrootd-hdfs/xrootd_hdfs_envcheck
```

and look in the output for updated values of `LD_LIBRARY_PATH`, `CLASSPATH`, and `LIBHDFS_OPTS`.

For the OSG packaging of HDFS, this should all work smoothly; you may need to re-implement the
`xrootd_hdfs_envcheck` script if you want to port this plugin to a non-RHEL platform.
