%define name python-gammu
%define version 0.17
%define release 1

Summary:    Python module to communicate with mobile phones
Name:       %{name}
Version:    %{version}
Release:    %{release}
Source0:    %{name}-%{version}.tar.bz2
License:    GPL
%if %_vendor == "suse"
Group:      Hardware/Mobile
%else
Group:      Applications/Communications
%endif
Packager:   Michal Cihar <michal@cihar.com>
Vendor:     Michal Cihar <michal@cihar.com>
BuildRequires: gammu-devel >= 1.08.16
Url:        http://cihar.com/gammu/wammu
Buildroot:  %{_tmppath}/%name-%version-root
%py_requires -d

%description
This provides gammu module, that can work with any phone Gammu
supports - many Nokias, Siemens, Alcatel, ...

%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" python setup.py build

%install
python setup.py install --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES

%clean
rm -rf $RPM_BUILD_ROOT

%files -f INSTALLED_FILES
%defattr(-,root,root)
%doc README AUTHORS COPYING NEWS examples
