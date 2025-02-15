Name: xrootd-hdfs
Version: 2.3.1
Release: 1%{?dist}
Summary: HDFS plugin for xrootd

Group: System Environment/Development
License: BSD
URL: https://github.com/UW-Madison-HEP/xrootd-hdfs
Source0: %{name}-%{version}.tar.gz

%define xrootd_current_major 5
%define xrootd_current_minor 6
%define xrootd_next_major 6

BuildRequires: xrootd-server-devel >= 1:%{xrootd_current_major}
BuildRequires: xrootd-server-devel <  1:%{xrootd_next_major}
BuildRequires: xrootd-devel >= 1:%{xrootd_current_major}
BuildRequires: xrootd-devel <  1:%{xrootd_next_major}
Requires: xrootd-server >= 1:%{xrootd_current_major}.%{xrootd_current_minor}
Requires: xrootd-server <  1:%{xrootd_next_major}.0.0-1
BuildRequires: xrootd-server-devel >= 1:%{xrootd_current_major}
BuildRequires: xrootd-server-devel <  1:%{xrootd_next_major}
BuildRequires: xrootd-devel >= 1:%{xrootd_current_major}
BuildRequires: xrootd-devel <  1:%{xrootd_next_major}
Requires: xrootd-server >= 1:%{xrootd_current_major}.%{xrootd_current_minor}
Requires: xrootd-server <  1:%{xrootd_next_major}.0.0-1
BuildRequires: cmake
BuildRequires: /usr/include/hdfs.h
BuildRequires: java-11-devel
BuildRequires: jpackage-utils
BuildRequires: openssl-devel
BuildRequires: zlib-devel
Requires: hadoop-libhdfs >= 3.3.0
Requires: hadoop-libhdfs < 3.4.0

%package devel
Summary: Development headers for Xrootd HDFS plugin
Group: System Environment/Development

%description
%{summary}

%description devel
%{summary}

%prep
%setup -q

%build
sed -i 's|@devel@|%{version}|' src/XrdHdfs.cc
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
%cmake_build

%install
%cmake_install

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xrootd
sed -e "s#@LIBDIR@#%{_libdir}#" rpm/xrootd.sample.hdfs.cfg.in > $RPM_BUILD_ROOT%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig
install -m 0644 rpm/xrootd-hdfs.sysconfig $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig/xrootd-hdfs

mkdir -p $RPM_BUILD_ROOT%{_libexecdir}/xrootd-hdfs
install -m 0755 $RPM_BUILD_ROOT%{_bindir}/xrootd_hdfs_envcheck $RPM_BUILD_ROOT%{_libexecdir}/xrootd-hdfs
rm $RPM_BUILD_ROOT%{_bindir}/xrootd_hdfs_envcheck

# Notice that I don't call ldconfig in post/postun.  This is because libXrdHdfs
# is really a loadable module, not a shared lib: it's not linked to all the xrootd
# libs necessary to load it outside xrootd.

%files
%defattr(-,root,root,-)
%{_libdir}/libXrdHdfs-*.so
%{_libdir}/libXrdHdfs-*.so*
%{_libdir}/libXrdHdfsReal-*.so
%{_sysconfdir}/xrootd/xrootd.sample.hdfs.cfg
%{_libexecdir}/xrootd-hdfs/xrootd_hdfs_envcheck
%config(noreplace) %{_sysconfdir}/sysconfig/xrootd-hdfs
%config %{_sysconfdir}/xrootd/config.d/40-xrootd-hdfs.cfg

%files devel
%{_includedir}/XrdHdfs.hh

%changelog
* Fri Feb 07 2025 Carl Vuosalo <covuosalo@wisc.edu> - 2.3.1-1
- Fix user name bug that caused gfal-rename to fail.

* Wed Feb 14 2024 Chad Seys <cwseys@physics.wisc.edu> - 2.3.0-3
- new location of libjvm.so for java-11 in EL8/9
- use new cmake macros as indicated by
  https://src.fedoraproject.org/rpms/conky/c/91d2bb5b96ad54446f33578535c466e1086333b9
  and https://fedoraproject.org/wiki/Changes/CMake_to_do_out-of-source_builds

* Wed Feb 14 2024 Chad Seys <cwseys@physics.wisc.edu> - 2.3.0-2
- build require java-11-devel
- explicitly require java-11.  It's available in EL7/8/9.

* Thu Feb 01 2024 Chad Seys <cwseys@physics.wisc.edu> - 2.3.0-1
- Fix runtime errors due to changing C/C++ compiler behavior. Thanks to Carl Vuosalo and Dan Bradley
- Bump xrootd version to 5.6.
- Improve java version build depends
- make libXrdHdfs globbing compatible with EL8

* Tue Feb 28 2023 Carl Vuosalo <covuosalo@wisc.edu> - 2.2.1
- Find correct user name when token is used.
- Advertize that AIO is not supported.
- Add Apache license.

* Thu Mar 11 2021 Carl Edquist <edquist@cs.wisc.edu> - 2.2.0-1
- Build against xrootd 5 (SOFTWARE-3923, #32)
- Only write checksum file if hdfsCloseFile() was successful (#34)
- Add support for new extended attribute API for XrdSecEntity (#35)

* Wed Sep 02 2020 Carl Edquist <edquist@cs.wisc.edu> - 2.1.8-1
- Load versioned libXrdHdfsReal .so name (SOFTWARE-4245)

* Tue Jul 14 2020 Diego Davila <didavila@ucsd.edu> - 2.1.7-9
- updating XRootD adding minor version to requirements (SOFTWARE-4137)

* Fri Jun 26 2020 Diego Davila <didavila@ucsd.edu> - 2.1.7-8
- updating XRootD requirements to only the major version (SOFTWARE-4137)

* Wed Jun 10 2020 Diego Davila <didavila@ucsd.edu> - 2.1.7-7
- Adding XrootD major version to the shared file name
- building against XrootD-4.12.2 (software-4093)

* Fri Apr 24 2020 Edgar Fajardo <emfajard@ucsd.edu> - 2.1.7-6
- Rebuild for xrootd 4.12 (SOFTWARE-4063)

* Mon Oct 21 2019 Carl Edquist <edquist@cs.wisc.edu> - 2.1.7-2
- Rebuild for xrootd 4.11 (SOFTWARE-3830)

* Mon Oct 21 2019 Carl Edquist <edquist@cs.wisc.edu> - 2.1.7-1
- Enable checksum verification (SOFTWARE-3803)
- Add version requirements for RCs (SOFTWARE-3767)

* Fri Jul 26 2019 Diego Davila <didavila@ucsd.edu> - 2.1.6-1
- Adding 40-xrootd-hdfs.cfg to CMakeLists.txt (SOFTWARE-3535)
- Adding .travis.yml

* Fri Jul 26 2019 Diego Davila <didavila@ucsd.edu> - 2.1.5-1
- Adding directory config.d and file config.d/40-xrootd-hdfs.cfg (SOFTWARE-3535)
- Changing xrootd requirement to 4.9.0

* Tue Feb 05 2019 Brian Bockelman <brian.bockelman@cern.ch> - 2.1.4-1
- Recompute checksum on parse failure.
- Force flush before close to hew closer to open-to-close semantics.

* Tue Sep 11 2018 Brian Bockelman <bbockelm@cse.unl.edu> - 2.1.3-1
- Implement missing LFN2PFN methods, fixing checksum calculations when name
  translation is used.

* Wed Jul 25 2018 Brian Bockelman <bbockelm@cse.unl.edu> - 2.1.1-1
- Add missing build deps.

* Wed Jul 25 2018 Brian Bockelman <bbockelm@cse.unl.edu> - 2.1.0-1
- Add support for doing checksums on write.

* Tue Apr 03 2018 Edgar Fajardo <efajardo@physics.ucsd.edu> - 2.0.2-1
- Change the build requires so it can be built for 3.3 (SOFTWARE-3165)

* Thu Mar 08 2018 Brian Bockelman <bbockelm@cse.unl.edu> - 2.0.1-1
- Fix directory creation logic.

* Tue Mar 06 2018 Brian Bockelman <bbockelm@cse.unl.edu> - 2.0.0-1
- Add full support for writing files into HDFS.

* Thu Jan 18 2018 Carl Edquist <edquist@cs.wisc.edu> - 1.9.2-5
- Drop version conflict for xrootd (SOFTWARE-2682)

* Thu Nov 09 2017 Carl Edquist <edquist@cs.wisc.edu> - 1.9.2-4
- Update hadoop build requirement (SOFTWARE-2982)

* Tue Nov 07 2017 Carl Edquist <edquist@cs.wisc.edu> - 1.9.2-3
- Rename java7 dependencies (SOFTWARE-2991)
- Rebuild against hadoop 2.6.0+ (SOFTWARE-2906)

* Mon Aug 07 2017 Marian Zvada <marian.zvada@cern.ch> - 1.9.2-2
- bumped version while changing Conflicts directive to Requires xrootd 4.6.1

* Wed Aug 02 2017 Marian Zvada <marian.zvada@cern.ch> - 1.9.2-1
- Fixes a minor bug for reporting error codes when listing directories fails
- Previously, it was possible for the error state to leak between calls to libhdfs

* Sat Mar 25 2017 Brian Bockelman <bbockelm@cse.unl.edu> - 1.9.1-1
- Fix for listing empty directories.

* Fri Mar 24 2017 Brian Bockelman <bbockelm@cse.unl.edu> - 1.9.0-1
- Implement xrootd's autostat protocol.
- Implement per-user connection caching.

* Mon Feb 27 2017 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8.8-2
- Fix empty directory listing.

* Thu Jul 21 2016 Edgar Fajardo <efajardo@physics.ucsd.edu> - 1.8.8-1
- Use native libraries (SOFTWARE-2387)
- Built from Brian's git repo

* Thu Jul 21 2016 Edgar Fajardo <efajardo@physics.ucsd.edu> - 1.8.8-0.1
- Use native libraries (SOFTWARE-2387)

* Mon Feb 22 2016 Carl Edquist <edquist@cs.wisc.edu> - 1.8.7-2
- Rebuild against hadoop-2.0.0+1612 (SOFTWARE-2161)

* Wed Jan 20 2016 Carl Edquist <edquist@cs.wisc.edu> - 1.8.7-1
- EL7 build fixes (SOFTWARE-2162)

* Tue Jan 5 2016 Edgar Fajardo <efajardo@physics.ucsd.edu> - 1.8.6-1
- Add support for non-world-readable files.

-* Thu Sep 25 2014 Brian Bockelman <bbockelm@cse.unl.edu> - 1.8.5-1
-- Add support for writes.

* Tue Feb 24 2015 Edgar Fajardo <efajardo@physics.ucsd.edu> - 1.8.4-4
- Remove xrootd-compat-libs not necessary
- Removed xrootd4 requirements

* Tue Feb 24 2015 Edgar Fajardo <efajardo@physics.ucsd.edu> - 1.8.4-3
- Change requirements to xrootd and added xrootd-compat-libs

* Thu Jul 10 2014 Edgar Fajardo <efajardo@physics.ucsd.edu> - 1.8.4-2
- Recompiled using xrootd4 libraries.

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

