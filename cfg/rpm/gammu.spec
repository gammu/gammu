%define ver      0.85.5.0
%define rel      1

Summary: 	 Mobile phones tools for Unix (Linux) and Win32
Name: 		 gammu
Version: 	 %ver
Release: 	 %rel
License: 	 GPL
Group:		 Applications/Communications
Packager:	 Marcin Wiacek <marcin@mwiacek.com>
Source: 	 %name-%{version}.tar.gz
URL:		 http://www.mwiacek.com
Buildroot: 	 %{_tmppath}/%{name}-%{version}-root

%description
Gammu can do such things with cellular phones as making data calls,
updating the address book, changing calendar and ToDo entries, sending and
receiving SMS messages, loading and getting ring tones and pictures (different
types of logos), synchronizing time, enabling NetMonitor, managing WAP
settings and bookmarks and much more. Functions depend on the phone model.

%prep
%setup -q
%build
%install
rm -rf $RPM_BUILD_ROOT
%configure
make installlib INSTALL_DOC_DIR=%_defaultdocdir/gammu/ DESTDIR=$RPM_BUILD_ROOT
make clean

%files
%defattr(-,root,root)
%_bindir/*
%_libdir/*
#localisations:
/usr/share/gammu
%doc %_mandir/man1/*
%doc %_defaultdocdir/gammu

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Thu Jan 02 2003 Michal Cihar <michal@cihar.com>
- made it install in directories that are defined in rpm

* Sun Nov 10 2002 Marcin Wiacek <marcin@mwiacek.com>
- topnet.pl email no more available

* Sun Sep 30 2002 Marcin Wiacek <marcin-wiacek@topnet.pl>
- build system is now really working OK

* Sat Sep 15 2002 R P Herrold <herrold@owlriver.com>
- initial packaging
