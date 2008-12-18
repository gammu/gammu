%define name python-gammu
%define ver 0.28
%define rel 1
%define extension   bz2

%{!?__python: %define __python python}
%{!?python_sitearch: %define python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}

Summary:    Python module to communicate with mobile phones
Name:       %{name}
Version:    %{ver}
Release:    %{rel}
Source:     http://dl.cihar.com/%{name}/latest/%{name}-%{ver}.tar.%{extension}
License:    GPLv2
%if 0%{?suse_version}
Group:      Development/Libraries/Python
%else
Group:      Development/Languages
%endif
Vendor:         Michal Čihař <michal@cihar.com>
BuildRequires: gammu-devel >= 1.21.95 python-devel
Url:        http://cihar.com/gammu/wammu
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: python
%{?py_requires}

%description
This provides gammu module, that can work with any phone Gammu
supports - many Nokias, Siemens, Alcatel, ...

%prep
%setup -q

%build
CFLAGS="$RPM_OPT_FLAGS" %{__python} setup.py build

%install
rm -rf %buildroot
mkdir %buildroot
%{__python} setup.py install --skip-build --root=%buildroot --prefix=%{_prefix}
%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?fedora} || 0%{?rhel}
%{__python} setup.py install -O1 --skip-build --root=%buildroot --prefix=%{_prefix}
%endif

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%doc README AUTHORS COPYING ChangeLog examples
%python_sitearch/*

%changelog
* Fri Oct 24 2008 Michal Čihař <michal@cihar.com> - 0.27-1
- fixed according to Fedora policy

* Wed Oct  8 2008 michal@cihar.com
- various fixups of the package
