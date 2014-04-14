
Name: xrootd-hdfs
Version: 1.8.4
Release: 1%{?dist}
Summary: HDFS plugin for xrootd

Group: System Environment/Development
License: BSD
URL: https://github.com/bbockelm/xrootd-hdfs
# Generated from:
# git-archive master | gzip -7 > ~/rpmbuild/SOURCES/xrootd-hdfs.tar.gz
Source0: %{name}.tar.gz
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
BuildRequires: xrootd-devel >= 1:3.3.1
BuildRequires: xrootd-server-devel >= 1:3.3.1
BuildRequires: cmake
BuildRequires: hadoop-libhdfs >= 2.0.0+545-1.cdh4.1.1
BuildRequires: java7-devel
BuildRequires: jpackage-utils
Requires: hadoop-client >= 2.0.0+545-1.cdh4.1.1
Conflicts: xrootd < 3.0.3-1

%package devel
Summary: Development headers for Xrootd HDFS plugin
Group: System Environment/Development

%description
%{summary}

%description devel
%{summary}

%prep
%setup -q -c -n %{name}-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make VERBOSE=1 %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xrootd
sed -e "s#@LIBDIR@#%{_libdir}#" spec/xrootd.sample.hdfs.cfg.in > $RPM_BUILD_ROOT%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg

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
%{_libdir}/libXrdHdfs.so
%{_libdir}/libXrdHdfs.so.*
%{_libdir}/libXrdHdfsReal.so
%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg
%{_libexecdir}/xrootd-hdfs/xrootd_hdfs_envcheck
%config(noreplace) %{_sysconfdir}/sysconfig/xrootd-hdfs

%files devel
%{_includedir}/XrdHdfs.hh

%changelog
* Mon Apr 14 2014 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8.4-1
- Add Xrootd versioning information.
- Fix directory listing.

* Mon Jun 03 2013 Matyas Selmeci <matyas@cs.wisc.edu> - 1.8.3-12
- Change to sysconfig file to handle symlinked $JAVA_HOME properly on el5

* Fri May 31 2013 Matyas Selmeci <matyas@cs.wisc.edu> - 1.8.3-11
- Add hadoop-client dependency

* Tue May 28 2013 Matyas Selmeci <matyas@cs.wisc.edu> - 1.8.3-10
- Fix segfault when LD_LIBRARY_PATH exists but is empty

* Thu May 23 2013 Matyas Selmeci <matyas@cs.wisc.edu> - 1.8.3-9
- Rebuild for hadoop 2.0; add sysconfig file

* Thu Apr 18 2013 Carl Edquist <edquist@cs.wisc.edu> - 1.8.3-8
- Merge in xrootd 3.3.1 reqs

* Thu Apr 18 2013 Matyas Selmeci <matyas@cs.wisc.edu> - 1.8.3-7
- Explicitly Require and BuildRequire xrootd 3.3.1

* Thu Apr 04 2013 Carl Edquist <edquist@cs.wisc.edu> - 1.8.3-5
- Rebuild for updated build dependency (hadoop-0.20),
- Explicitly add java7-devel build requirement

* Wed Apr 03 2013 Matyas Selmeci <matyas@cs.wisc.edu> - 1.8.3-4
- Bump to rebuild against xrootd 3.3.1
- Rename xrootd-libs-devel dependency to match what 3.3.1 calls it

* Mon Nov 26 2012 Doug Strain <dstrain@fnal.gov> - 1.8.3-3
- Rebuild to link correctly with libhdfs

* Mon Nov 19 2012 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8.3-1
- Fix symbol visibility issues with xrootd 3.3.0-rc1.

* Tue Nov 13 2012 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8.2-1
- Fix compilation issue in mock.

* Sun Nov 11 2012 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8.1-1
- Minor tweaks for 3.3.0 RC.

* Mon Oct 22 2012 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8-1
- Changeover to cmake.
- Rebuild for xrootd-3.3; drop usage of private headers.
- Split off -devel RPM.

* Sun Dec 11 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.7.1-1
- Fix logging line for readahead.

* Tue Nov 29 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.7.0-1
- Add read-ahead buffer; contribution from Dan Bradley.

* Tue Oct 25 2011 Matevz Tadel <mtadel@ucsd.edu> 1.6.0-1
- Updated for xrootd-3.1.
- Compatibility support for hadoop 0.19.

* Mon Aug 29 2011 Brian Bockelman <bbockelm@cse.unl.edu> - 1.5.0-2
- New build for OSG koji.

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

