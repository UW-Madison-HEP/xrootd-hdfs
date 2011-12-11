
Name: xrootd-hdfs-compat
Version: 1.7.1
Release: 1
Summary: HDFS plugin for xrootd

Group: System Environment/Daemons
License: BSD
URL: svn://t2.unl.edu/brian/XrdHdfs
Source0: xrootd-hdfs-%{version}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: xrootd-server-devel >= 3.1 hadoop-fuse-devel >= 0.19
Requires: xrootd-server >= 3.1 hadoop-fuse >= 0.19 hadoop >= 0.19
Conflicts: xrootd < 3.0.3-1

%description
%{summary}

%prep
%setup -q -n xrootd-hdfs-%{version}

%build
LDFLAGS="-L/usr/java/default/jre/lib/amd64 -L/usr/java/default/jre/lib/amd64/server -L/usr/java/default/jre/lib/i386/server -L/usr/java/default/jre/lib/i386"
%ifarch x86_64
%configure --with-xrootd-incdir=/usr/include/xrootd --with-jvm-incdir=/usr/java/default/include --with-jvm-libdir=/usr/java/default/jre/lib/amd64/server
%else
%configure --with-xrootd-incdir=/usr/include/xrootd --with-jvm-incdir=/usr/java/default/include --with-jvm-libdir=/usr/java/default/jre/lib/i386/server
%endif
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xrootd
sed -e "s#@LIBDIR@#%{_libdir}#" spec/xrootd.sample.hdfs.cfg.in > $RPM_BUILD_ROOT%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg
rm -rf $RPM_BUILD_ROOT%{_libdir}/*.a
rm -rf $RPM_BUILD_ROOT%{_libdir}/*.la

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig
install -m 0644 spec/xrootd-hdfs-compat.sysconfig $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig/xrootd-hdfs

mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/xrootd-hdfs
install -m 0755 $RPM_BUILD_ROOT%{_bindir}/xrootd_hdfs_envcheck $RPM_BUILD_ROOT%{_libexecdir}/xrootd-hdfs
rm $RPM_BUILD_ROOT%{_bindir}/xrootd_hdfs_envcheck

# Notice that I don't call ldconfig in post/postun.  This is because libXrdHdfs
# is really a loadable module, not a shared lib: it's not linked to all the xrootd
# libs necessary to load it outside xrootd.

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/libXrdHdfs*
%{_includedir}/xrootd/XrdHdfs/XrdHdfs.hh
%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg
%{_libexecdir}/xrootd-hdfs/xrootd_hdfs_envcheck
%config(noreplace) %{_sysconfdir}/sysconfig/xrootd-hdfs

%changelog
* Sun Dec 11 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.7.1-1
- Fix logging line for readahead.

* Tue Nov 29 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.7.0-1
- Add read-ahead buffer; contribution from Dan Bradley.

* Tue Oct 25 2011 Matevz Tadel <mtadel@ucsd.edu> 1.6.0-1
- Clone from xrootd-hdfs to provide legacy support for hadoop-0.19.
