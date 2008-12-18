%define ver         1.22.0
%define name        gammu
%define rel         1
# Set to 0 to disable bluetooth support
%if 0%{?opensuse_bs} && 0%{?sles_version} == 9
%define bluetooth   0
%else
%define bluetooth   1
%endif
# Set to 0 to disable PostgreSQL support
%define pqsql     1
# Set to 0 to disable MySQLSQL support
%define mysql     1
# Change if using tar.gz sources
%define extension   bz2

%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
%define gammu_docdir %_docdir/%name-%ver
%else
%define gammu_docdir %_docdir/%name
%endif

Summary:            Mobile phone management utility
Name:               %name
Version:            %ver
Release:            %rel
License:            GPLv2
%if 0%{?suse_version}
Group:              Hardware/Mobile
%else
Group:              Applications/Communications
%endif
Vendor:         Michal Čihař <michal@cihar.com>

# Detect build requires
# I hate this crap
%if 0%{?suse_version}

# SUSE
%define dist_bluez_libs bluez-libs >= 2.0
%if 0%{?suse_version} >= 1030
%define dist_pkgconfig pkg-config
%define dist_mysql_libs libmysqlclient-devel 
%else 
%define dist_pkgconfig pkgconfig
%define dist_mysql_libs mysql-devel 
%endif

%else

%if 0%{?mandriva_version}

# Mandriva
%define dist_pkgconfig pkgconfig
%ifarch x86_64
%define mandriva_hack 64
%endif
%if 0%{?mandriva_version} > 2007
%define dist_mysql_libs lib%{?mandriva_hack}mysql-devel
%else
%define dist_mysql_libs lib%{?mandriva_hack}mysql15-devel
%endif
%if 0%{?mandriva_version} > 2007
%define dist_bluez_libs lib%{?mandriva_hack}bluez2 lib%{?mandriva_hack}bluez-devel
%else
%if 0%{?mandriva_version} > 2006
%define dist_bluez_libs lib%{?mandriva_hack}bluez2 lib%{?mandriva_hack}bluez2-devel
%else
%define dist_bluez_libs libbluez1 >= 2.0 libbluez1-devel >= 2.0
%endif
%endif

%else

# Fedora
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
%define dist_pkgconfig pkgconfig
%if 0%{?fedora_version} >= 8 || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} >= 8 || 0%{?rhel}
%define dist_mysql_libs mysql-devel 
%else
%define dist_mysql_libs mysqlclient14-devel
%endif
%define dist_bluez_libs bluez-libs >= 2.0 bluez-libs-devel >= 2.0

%else

#Defaults
%define dist_pkgconfig pkg-config
%define dist_mysql_libs libmysqlclient-devel 
%define dist_bluez_libs bluez-libs >= 2.0 bluez-libs-devel >= 2.0

%endif
%endif
%endif

%if %bluetooth
BuildRequires: %{dist_bluez_libs}
%endif

%if pqsql
BuildRequires: postgresql-devel
%endif

%if %mysql
BuildRequires: %{dist_mysql_libs}
%endif

BuildRequires: gettext cmake %{dist_pkgconfig}

Source:         http://dl.cihar.com/gammu/releases/gammu-%{ver}.tar.%{extension}
URL:            http://cihar.com/gammu/
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root

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

%package devel
Summary:      Development files for Gammu
%if 0%{?suse_version}
Group:              Development/Libraries/C and C++
%else
Group:              Development/Libraries
%endif
Autoreqprov:  on
Requires:           %name = %ver-%release %{dist_pkgconfig}

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

%prep
%setup -q

%build
mkdir build-dir
cd build-dir
cmake ../ \
    -DENABLE_SHARED=ON \
    -DCMAKE_INSTALL_PREFIX=%_prefix \
    -DINSTALL_DOC_DIR=%gammu_docdir \
    -DINSTALL_LIB_DIR=%_lib \
    -DINSTALL_LIBDATA_DIR=%_lib
make %{?_smp_mflags} %{!?_smp_mflags:%{?jobs:-j %jobs}}

%check
make -C build-dir test

%install
rm -rf %buildroot
mkdir %buildroot
make -C build-dir install DESTDIR=%buildroot
%find_lang %{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files -f %name.lang
%defattr(-,root,root)
%_bindir/*
%_libdir/*.so.*
%doc %_mandir/man1/*
%doc %gammu_docdir

%files devel
%defattr(-,root,root)
%_includedir/%name
%_libdir/pkgconfig/%name.pc
%_libdir/*.so

%clean
rm -rf %buildroot

%changelog
* Fri Oct 24 2008 Michal Čihař <michal@cihar.com> - 1.21.0-1
- fixed according to Fedora policy

* Wed Oct  8 2008  Michal Cihar <michal@cihar.com>
- do not remove build root in %%install
- move make test to %%check

* Tue Oct  7 2008  Michal Cihar <michal@cihar.com>
- use find_lang macro

* Thu Mar 28 2007  Michal Cihar <michal@cihar.com>
- update to current code status

* Thu Jan  6 2005  Michal Cihar <michal@cihar.com>
- add support for Mandrake, thanks to Olivier BERTEN <Olivier.Berten@advalvas.be> for testing
- use new disable-bluetooth

* Wed Nov 12 2003 Michal Cihar <michal@cihar.com>
- distiguish between packaging on SUSE and Redhat
- build depends on bluez if wanted

* Mon Nov 10 2003 Peter Soos <sp@osb.hu>
- using rpm macros where is possible
- added ldconfig to post/postun

* Mon Nov 03 2003 Michal Cihar <michal@cihar.com>
- split devel package

* Thu Jan 02 2003 Michal Cihar <michal@cihar.com>
- made it install in directories that are defined in rpm

* Sun Nov 10 2002 Marcin Wiacek <marcin@mwiacek.com>
- topnet.pl email no more available

* Sun Sep 30 2002 Marcin Wiacek <marcin-wiacek@topnet.pl>
- build system is now really working OK

* Sat Sep 15 2002 R P Herrold <herrold@owlriver.com>
- initial packaging
