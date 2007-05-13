%define ver         1.11.90
%define name        gammu
%define relnum      1

%if %_vendor == "suse"
%define rel         %{relnum}suse
%else
%if %_vendor == "redhat"
%define rel         %{relnum}rh
%else
%if %_vendor == "MandrakeSoft"
%define rel         %{relnum}mdk
%else
%define rel         %{relnum}
%endif
%endif
%endif

%if %_vendor == "redhat"
%define gammu_docdir %_docdir/%name-%ver
%else
%define gammu_docdir %_docdir/%name
%endif

Summary:            Mobile phones tools for Unix (Linux) and Win32
Name:               %name
Version:            %ver
Release:            %rel
License:            GPL
%if %_vendor == "suse"
Group:              Hardware/Mobile
%else
Group:              Applications/Communications
%endif
%if %bluetooth
%if %_vendor == "suse"
BuildRequires:      bluez-libs >= 2.0 gettext cmake
%else
%if %_vendor == "MandrakeSoft"
BuildRequires:      libbluez1 >= 2.0 libbluez1-devel >= 2.0 gettext cmake
%else
BuildRequires:      bluez-libs >= 2.0 bluez-libs-devel >= 2.0 gettext cmake
%endif
%endif
%endif
Vendor:             Michal Cihar <michal@cihar.com>
Source:             http://dl.cihar.com/gammu/releases/gammu-%{ver}.tar.bz2
URL:                http://cihar.com/gammu/
Buildroot:          %{_tmppath}/%name-%version-root

%description
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

%package devel
Summary:      Development files for Gammu
%if %_vendor == "suse"
Group:              Development/Libraries/C and C++
%else
Group:              Development/Libraries
%endif
Autoreqprov:  on
Requires:           %name = %ver-%rel

%description devel
Gammu is command line utility and library to work with mobile phones
from many vendors. Support for different models differs, but basic
functions should work with majority of them. Program can work with
contacts, messages (SMS, EMS and MMS), calendar, todos, filesystem,
integrated radio, camera, etc. It also supports daemon mode to send and
receive SMSes.

This package contain files needed for development.

%prep
%setup -q

%build
mkdir build-dir
cd build-dir && cmake ../ -DENABLE_SHARED=ON -DCMAKE_INSTALL_PREFIX=%_prefix -DINSTALL_DOC_DIR=%_docdir/%name
make -C build-dir

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
