
Name: xrootd-hdfs
Version: 1.6.0
Release: 1
Summary: HDFS plugin for xrootd

Group: System Environment/Daemons
License: BSD
URL: svn://t2.unl.edu/brian/XrdHdfs
Source0: %{name}-%{version}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: xrootd-server-devel >= 3.1 hadoop-0.20-libhdfs >= 0.20.2+737-4
Requires: xrootd-server >= 3.1 hadoop-0.20-libhdfs >= 0.20.2+737-4 hadoop-0.20 >= 0.20.2+737-4
Conflicts: xrootd < 3.0.3-1

%description
%{summary}

%prep
%setup -q

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
install -m 0644 spec/xrootd-hdfs.sysconfig $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig/xrootd-hdfs

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
* Tue Oct 25 2011 Matevz Tadel <mtadel@ucsd.edu> 1.6.0-1
- Updated for xrootd-3.1.
- Compatibility support for hadoop 0.19.

* Fri Jun 3 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.5.0-1
- More reliable, if kludgy, Hadoop environment setup.

* Tue May 17 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.9-1
- Fix directory listing segfault

* Tue May 17 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.8-2
- Fix logic error in the environment setup script.

* Thu Mar 24 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.8-1
- Add profile.d file to get the xrootd binary's environment correct.

* Thu Mar 24 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.7-2
- Update RPM dependencies for new upstream xrootd package names.

* Mon Mar 14 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.7-1
- Ship our own sysconfig script; remove the need for xrootd wrapper scripts.

* Mon Mar  7 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.6-1
- Review to make sure all return values are non-positive.

* Fri Feb 25 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.5-2
- Fix a double-close of the filesystem.

* Thu Feb 24 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.5-1
- Cleanup filesystem handles after use.

* Tue Feb 1 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.4-1
- Update for new HDFS headers.
- Login everyone as user "nobody", instead of the "fake" xrootd username.
  Necessary for compatibility with KRB5-enabled HDFS.

* Mon Jan 31 2011 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.3-3
- Fix the sample configuration.  Thanks to Will Maier.

* Thu Dec 23 2010 Brian Bockelman <bbockelm@cse.unl.edu> 1.4.3-2
- Switch to a Hadoop-0.20 build.

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

