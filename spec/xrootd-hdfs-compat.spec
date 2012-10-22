
Name: xrootd-hdfs-compat
Version: 1.8
Release: 1
Summary: HDFS plugin for xrootd

Group: System Environment/Development
License: BSD
URL: https://github.com/bbockelm/xrootd-hdfs
# Generated from:
# git-archive master | gzip -7 > ~/rpmbuild/SOURCES/xrootd-lcmaps.tar.gz
Source0: xrootd-hdfs-%{version}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: xrootd-server-devel >= 3.1 hadoop-fuse-devel >= 0.19
Requires: xrootd-server >= 3.1 hadoop-fuse >= 0.19 hadoop >= 0.19
Conflicts: xrootd < 3.0.3-1

%package devel
Summary: Development headers and libraries for Xrootd HDFS plugin
Group: System Environment/Development

%description
%{summary}

%description devel
%{summary}

%prep
%setup -q -c -n xrootd-hdfs-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make VERBOSE=1 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xrootd
sed -e "s#@LIBDIR@#%{_libdir}#" spec/xrootd.sample.hdfs.cfg.in > $RPM_BUILD_ROOT%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg

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
%{_libdir}/libXrdHdfs.so.*
%{_libdir}/libXrdHdfsReal.so
%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg
%{_libexecdir}/xrootd-hdfs/xrootd_hdfs_envcheck
%config(noreplace) %{_sysconfdir}/sysconfig/xrootd-hdfs

%files devel
%{_includedir}/XrdHdfs.hh
%{_libdir}/libXrdHdfs.so

%changelog
* Mon Oct 22 2012 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8-1
- Changeover to cmake.
- Rebuild for xrootd-3.3; drop usage of private headers.
- Split off -devel RPM.

* Sun Dec 11 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.7.1-1
- Fix logging line for readahead.

* Tue Nov 29 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.7.0-1
- Add read-ahead buffer; contribution from Dan Bradley.

* Tue Oct 25 2011 Matevz Tadel <mtadel@ucsd.edu> 1.6.0-1
- Clone from xrootd-hdfs to provide legacy support for hadoop-0.19.
