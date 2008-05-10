%define ver         1.20.0
%define name        gammu
%define rel         1
# Set to 0 to disable bluetooth support
%if 0%{?opensuse_bs} && 0%{?sles_version} == 9
%define bluetooth   0
%else
%define bluetooth   1
%endif
# Set to 0 to disable PostgreSQL support
%define pqsql	    1
# Set to 0 to disable MySQLSQL support
%define mysql	    1
# Change if using tar.gz sources
%define extension   bz2

%if 0%{?fedora_version}
%define gammu_docdir %_docdir/%name-%ver
%else
%define gammu_docdir %_docdir/%name
%endif

Summary:            Mobile phone management utility
Name:               %name
Version:            %ver
Release:            %rel
License:            GPL
%if 0%{?suse_version}
Group:              Hardware/Mobile
%else
Group:              Applications/Communications
%endif

# Detect build requires
# I hate this crap
%if 0%{?suse_version}

# SUSE
%define dist_bluez_libs	bluez-libs >= 2.0
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
%define dist_mysql_libs libmysql15-devel
%if 0%{?mandriva_version} > 2006
%define dist_bluez_libs	libbluez2 libbluez2-devel
%else
%define dist_bluez_libs	libbluez1 >= 2.0 libbluez1-devel >= 2.0
%endif

%else

# Fedora
%if 0%{?fedora_version}
%define dist_pkgconfig pkgconfig
%if 0%{?fedora_version} >= 8
%define dist_mysql_libs mysql-devel 
%else
%define dist_mysql_libs mysqlclient14-devel
%endif
%define dist_bluez_libs	bluez-libs >= 2.0 bluez-libs-devel >= 2.0

%else

#Defaults
%define dist_pkgconfig pkg-config
%define dist_mysql_libs libmysqlclient-devel 
%define dist_bluez_libs	bluez-libs >= 2.0 bluez-libs-devel >= 2.0

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

Vendor:             Michal Cihar <michal@cihar.com>
Source:             http://dl.cihar.com/gammu/releases/gammu-%{ver}.tar.%{extension}
URL:                http://cihar.com/gammu/
Buildroot:          %{_tmppath}/%name-%version-root

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
make
make test

%install
rm -rf %buildroot
make -C build-dir install DESTDIR=%buildroot

%post
if test -f /etc/ld.so.conf ; then
    /sbin/ldconfig
fi

%postun
if test -f /etc/ld.so.conf ; then
    /sbin/ldconfig
fi

%files
%defattr(-,root,root)
%_bindir/*
%_libdir/*.so.*
#localisations:
/usr/share/locale
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
