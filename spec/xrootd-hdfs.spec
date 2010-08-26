
Name: xrootd-hdfs
Version: 1.4.0
Release: 2
Summary: HDFS plugin for xrootd

Group: System Environment/Daemons
License: BSD
URL: svn://t2.unl.edu/brian/XrdHdfs
Source0: %{name}-%{version}.tar.gz
Source1: xrootd.sample.hdfs.cfg.in
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: xrootd-devel hadoop-fuse-devel
Requires: xrootd hadoop-fuse

%description
%{summary}

%prep
%setup -q

%build
LDFLAGS="-L/usr/java/default/jre/lib/amd64 -L/usr/java/default/jre/lib/amd64/server -L/usr/java/default/jre/lib/i386/server -L/usr/java/default/jre/lib/i386"
%configure --with-xrootd-incdir=/usr/include/xrootd
make

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

sed -e "s#@LIBDIR@#%{_libdir}#" %{SOURCE2} > $RPM_BUILD_ROOT%{_sysconfdir}/%{name}
/xrootd.sample.hdfs.cfg

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%{_libdir}/libXrdHdfs*
%{_includedir}/xrootd/XrdHdfs/XrdHdfs.hh
%{_sysconfdir}/%{name}/xrootd.sample.dcap.cfg

%changelog
* Tue Aug 24 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.0-1
- Break xrootd-hdfs off into its own standalone RPM.

