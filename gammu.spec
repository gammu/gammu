#
# spec file for package gammu
#


%define so_ver 8

# Change if using tar.gz or tar.bz2 sources
%define extension   xz

Name:           gammu
Version:        1.42.0
Release:        0
Summary:        Mobile phone management utility
License:        GPL-2.0
%if 0%{?suse_version}
Group:          Hardware/Mobile
%else
Group:          Applications/Communications
%endif
Url:            https://wammu.eu/gammu/
Source0:        https://dl.cihar.com/gammu/releases/%{name}-%{version}.tar.%{extension}

# Set to 0 to disable PostgreSQL support
%define pqsql     1
# Set to 0 to disable MySQL support
%define mysql     1
# Set to 0 to disable DBI support
%define dbi       1
# Set to 0 to disable ODBC support
%define odbc      1
# Set to 0 to disable USB support
%define usb       1
# Set to 0 to disable bluetooth support
%if 0%{?opensuse_bs} && 0%{?sles_version} == 9
%define bluetooth   0
%else
%define bluetooth   1
%endif
# Docdir
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
%define gammu_docdir %{_docdir}/%{name}-%{version}
%else
%define gammu_docdir %{_docdir}/%{name}
%endif
# Detect build requires
# SUSE
%if 0%{?suse_version}
%define dist_usb_libs libusb-1_0-devel
%define dist_dbi_libs libdbi-devel libdbi-drivers-dbd-sqlite3 sqlite3
%define dist_bluez_libs bluez-devel
%define dist_postgres_libs postgresql-devel
%else
# Mandriva
%if 0%{?mandriva_version}
# 64-bit Mandriva has 64 in package name
%ifarch x86_64
%define mandriva_hack 64
%endif
%if 0%{?mandriva_version} > 200910
%define dist_usb_libs lib%{?mandriva_hack}usb1.0-devel
%else
%define dist_usb_libs lib%{?mandriva_hack}usb-devel
%endif
%if 0%{?mandriva_version} > 2010
%define dist_dbi_libs lib%{?mandriva_hack}dbi-devel libdbi-drivers-dbd-sqlite3 sqlite3-tools
%endif
%define dist_bluez_libs lib%{?mandriva_hack}bluez-devel
# postgresql-devel does not work for whatever reason in buildservice
%if 0%{?mandriva_version} == 2009
%define dist_postgres_libs postgresql8.3-devel
%else
%define dist_postgres_libs postgresql-devel
%endif
%else
# Fedora / Redhat / Centos
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel} || 0%{?scilin_version}
%if 0%{?fedora_version} || 0%{?centos_version} >= 700 || 0%{?rhel_version} >= 700 || 0%{?scilin_version} >= 700
%define dist_usb_libs libusbx-devel
%else
%define dist_usb_libs libusb1-devel
%endif
%define dist_dbi_libs libdbi-devel libdbi-dbd-sqlite sqlite
%define dist_bluez_libs bluez-libs-devel >= 2.0
%define dist_postgres_libs postgresql-devel
%else
# Defaults for not known distributions
%define dist_usb_libs libusb1-devel
%define dist_dbi_libs libdbi-devel libdbi-dbd-sqlite sqlite
%define dist_bluez_libs bluez-libs-devel >= 2.0
%define dist_postgres_libs postgresql-devel
%endif
%endif
%endif
BuildRequires:  cmake >= 3.0
BuildRequires:  gcc
BuildRequires:  gettext
BuildRequires:  pkgconfig
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
%if %{bluetooth}
BuildRequires:  %{dist_bluez_libs}
%endif
%if %{pqsql}
BuildRequires:  %{dist_postgres_libs}
%endif
%if %{mysql}
BuildRequires:  mysql-devel
%endif
%if %{dbi}
BuildRequires:  %{dist_dbi_libs}
%endif
%if %{odbc}
BuildRequires:  unixODBC-devel
%endif
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
BuildRequires:  glib2-devel
BuildRequires:  libgudev1-devel
%else
%if 0%{?mandriva_version}
BuildRequires:  glib2-devel
BuildRequires:  libgudev1.0-devel
%else
BuildRequires:  glib2-devel
BuildRequires:  libgudev-1_0-devel
%endif
%endif
%if 0%{?suse_version} > 1310 || 0%{?fedora_version} >= 15 || 0%{?centos_version} >= 700 || 0%{?rhel_version} >= 700 || 0%{?scientificlinux_version} >= 600
BuildRequires:  systemd
%endif
%if 0%{?centos_version} || 0%{?rhel_version} || 0%{?rhel} || 0%{?suse_version} < 1100
BuildRequires:  curl-devel
%else
BuildRequires:  libcurl-devel
%endif
%if %{usb}
BuildRequires:  %{dist_usb_libs}
%endif

%description
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

Currently supported phones include:

* Many Nokia models.
* Alcatel BE5 (501/701), BF5 (715), BH4 (535/735).
* AT capable phones (Siemens, Nokia, Alcatel, IPAQ).
* OBEX and IrMC capable phones (Sony-Ericsson, Motorola).
* Symbian phones through gnapplet.

This package contains Gammu binary as well as some examples.

%package devel
Summary:        Development files for Gammu
%if 0%{?suse_version}
Group:          Development/Libraries/C and C++
%else
Group:          Development/Libraries
%endif
Requires:       libGammu%{so_ver} = %{version}
Requires:       libgsmsd%{so_ver} = %{version}

%description devel
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

Currently supported phones include:

* Many Nokia models.
* Alcatel BE5 (501/701), BF5 (715), BH4 (535/735).
* AT capable phones (Siemens, Nokia, Alcatel, IPAQ).
* OBEX and IrMC capable phones (Sony-Ericsson, Motorola).
* Symbian phones through gnapplet.

This package contain files needed for development.

%package smsd
Summary:        SMS message daemon
%if 0%{?suse_version}
# FIXME: use proper Requires(pre/post/preun/...)
PreReq:         %insserv_prereq  %fillup_prereq
%endif
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
Requires(post): chkconfig
Requires(preun): chkconfig
Requires(preun): initscripts
%endif
%if 0%{?suse_version}
Group:          Hardware/Mobile
%else
Group:          Applications/Communications
%endif

%description smsd
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

Currently supported phones include:

* Many Nokia models.
* Alcatel BE5 (501/701), BF5 (715), BH4 (535/735).
* AT capable phones (Siemens, Nokia, Alcatel, IPAQ).
* OBEX and IrMC capable phones (Sony-Ericsson, Motorola).
* Symbian phones through gnapplet.

This package contains the Gammu SMS Daemon and tool to inject messages
into the queue.

%package -n libGammu%{so_ver}
Summary:        Mobile phone management library
Group:          System/Libraries

%description -n libGammu%{so_ver}
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

Currently supported phones include:

* Many Nokia models.
* Alcatel BE5 (501/701), BF5 (715), BH4 (535/735).
* AT capable phones (Siemens, Nokia, Alcatel, IPAQ).
* OBEX and IrMC capable phones (Sony-Ericsson, Motorola).
* Symbian phones through gnapplet.

This package contains the Gammu shared library.

%package -n libgsmsd%{so_ver}
Summary:        SMS daemon helper library
Group:          System/Libraries

%description -n libgsmsd%{so_ver}
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

Currently supported phones include:

* Many Nokia models.
* Alcatel BE5 (501/701), BF5 (715), BH4 (535/735).
* AT capable phones (Siemens, Nokia, Alcatel, IPAQ).
* OBEX and IrMC capable phones (Sony-Ericsson, Motorola).
* Symbian phones through gnapplet.

This package contains the Gammu SMS daemon shared library.

%prep
%setup -q

%build
mkdir build
cd build
export CFLAGS="%{optflags}"
export CXXFLAGS="%{optflags}"
cmake ../ \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=%{_prefix} \
    -DINSTALL_DOC_DIR=%{gammu_docdir} \
    -DINSTALL_LIB_DIR=%{_lib} \
    -DINSTALL_LIBDATA_DIR=%{_lib} \
    -DINSTALL_LSB_INIT=ON \
    -DINSTALL_UDEV_RULES=ON \
    -DSYSTEMD_FOUND=ON \
    -DWITH_SYSTEMD=ON
make %{?_smp_mflags} VERBOSE=1

%check
cd build
ctest -V
cd ..

%install
make -C build install DESTDIR=%{buildroot}

# Install config file
install -pm 0644 docs/config/smsdrc %{buildroot}%{_sysconfdir}/gammu-smsdrc

# Move init script to correct location
if [ /etc/init.d != %{_initddir} ] ; then
    install -d -m 0755 %{buildroot}%{_initddir}
    mv %{buildroot}/etc/init.d/gammu-smsd %{buildroot}%{_initddir}
fi

%find_lang %{name}
%find_lang libgammu

%post -n libGammu%{so_ver} -p /sbin/ldconfig

%postun -n libGammu%{so_ver} -p /sbin/ldconfig

%post -n libgsmsd%{so_ver} -p /sbin/ldconfig

%postun -n libgsmsd%{so_ver} -p /sbin/ldconfig

%post smsd
%if 0%{?mandriva_version}
%{_post_service} gammu-smsd
%endif
%if 0%{?suse_version}
%fillup_and_insserv gammu-smsd
%endif
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
/sbin/chkconfig --add gammu-smsd
%endif

%preun smsd
%if 0%{?suse_version}
%stop_on_removal gammu-smsd
%endif
%if 0%{?mandriva_version}
%{_preun_service} gammu-smsd
%endif
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
if [ $1 = 0 ] ; then
    /sbin/service gammu-smsd stop >/dev/null 2>&1
    /sbin/chkconfig --del gammu-smsd
fi
%endif


%postun smsd
%if 0%{?suse_version}
%restart_on_update gammu-smsd
%insserv_cleanup
%endif


%files -f %{name}.lang
%defattr(-,root,root,-)
%config %{_sysconfdir}/bash_completion.d/gammu
%{_bindir}/gammu
%{_bindir}/gammu-config
%{_bindir}/gammu-detect
%{_bindir}/jadmaker
%doc %{gammu_docdir}
%doc %{_mandir}/man1/gammu-config.1*
%doc %{_mandir}/man1/gammu-detect.1*
%doc %{_mandir}/man1/gammu.1*
%doc %{_mandir}/man1/jadmaker.1*
%doc %{_mandir}/man5/gammu-backup.5*
%doc %{_mandir}/man5/gammu-smsbackup.5*
%doc %{_mandir}/man5/gammurc.5*

%files devel
%defattr(-,root,root,-)
%{_includedir}/gammu/
%{_libdir}/*.so
%{_libdir}/pkgconfig/gammu-smsd.pc
%{_libdir}/pkgconfig/gammu.pc

%files smsd
%defattr(-,root,root)
%attr(755,root,root) %config %{_initddir}/gammu-smsd
%config %{_sysconfdir}/gammu-smsdrc
%{_bindir}/gammu-smsd
%{_bindir}/gammu-smsd-inject
%{_bindir}/gammu-smsd-monitor
%doc %{_mandir}/man1/gammu-smsd-inject.1*
%doc %{_mandir}/man1/gammu-smsd-monitor.1*
%doc %{_mandir}/man1/gammu-smsd.1*
%doc %{_mandir}/man5/gammu-smsdrc.5*
%doc %{_mandir}/man7/gammu-smsd-dbi.7*
%doc %{_mandir}/man7/gammu-smsd-files.7*
%doc %{_mandir}/man7/gammu-smsd-mysql.7*
%doc %{_mandir}/man7/gammu-smsd-null.7*
%doc %{_mandir}/man7/gammu-smsd-odbc.7*
%doc %{_mandir}/man7/gammu-smsd-pgsql.7*
%doc %{_mandir}/man7/gammu-smsd-run.7*
%doc %{_mandir}/man7/gammu-smsd-sql.7*
%doc %{_mandir}/man7/gammu-smsd-tables.7*
%dir %{_libexecdir}/systemd
%dir %{_libexecdir}/systemd/system
%{_libexecdir}/systemd/system/gammu-smsd.service

%files -n libGammu%{so_ver} -f libgammu.lang
%defattr(-,root,root,-)
%{_libdir}/libGammu.so.%{so_ver}*
%{_datadir}/gammu/
%dir /etc/udev
%dir /etc/udev/rules.d
/etc/udev/rules.d/69-gammu-acl.rules

%files -n libgsmsd%{so_ver}
%defattr(-,root,root,-)
%{_libdir}/libgsmsd.so.%{so_ver}*

%changelog
