
Name: xrootd-hdfs
Version: 1.4.3
Release: 1
Summary: HDFS plugin for xrootd

Group: System Environment/Daemons
License: BSD
URL: svn://t2.unl.edu/brian/XrdHdfs
Source0: %{name}-%{version}.tar.gz
Source1: xrootd.sample.hdfs.cfg.in
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: xrootd-devel >= 1.4 hadoop-fuse-devel
Requires: xrootd >= 1.4 hadoop-fuse

%description
%{summary}

%prep
%setup -q

%build
LDFLAGS="-L/usr/java/default/jre/lib/amd64 -L/usr/java/default/jre/lib/amd64/server -L/usr/java/default/jre/lib/i386/server -L/usr/java/default/jre/lib/i386"
%configure --with-xrootd-incdir=/usr/include/xrootd --with-jvm-incdir=/usr/java/default/include --with-jvm-libdir=/usr/java/default/jre/lib/amd64/server
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xrootd
sed -e "s#@LIBDIR@#%{_libdir}#" %{SOURCE1} > $RPM_BUILD_ROOT%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg
rm -rf $RPM_BUILD_ROOT%{_libdir}/*.a
rm -rf $RPM_BUILD_ROOT%{_libdir}/*.la

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/libXrdHdfs*
%{_includedir}/xrootd/XrdHdfs/XrdHdfs.hh
%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg

%changelog
* Mon Dec 6 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.3-1
- Update to fix an off-by-one error in the directory listing.

* Tue Nov 9 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.2-1
- Rebuild for updated xrootd.
- Remove libtool archive and static library.  Fix usage of makeinstall macro.

* Fri Sep 24 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.1-1
- Version bump in order to compile against latest xrootd.

* Thu Aug 26 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.0-2
- Add in the sample configuration file.

* Tue Aug 24 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.0-1
- Break xrootd-hdfs off into its own standalone RPM.

