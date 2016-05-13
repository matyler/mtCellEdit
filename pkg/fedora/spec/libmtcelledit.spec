Name:			libmtcelledit
Version:		@MT_VERSION@
Release:		@MT_RELVER@%{?dist}
Summary:		MT Spreadsheet Engine
License:		GPLv3+
Source:			%{name}-%{version}.tar.gz
BuildRequires:		zlib-devel libmtkit bison
Requires:		zlib libmtkit

%global debug_package %{nil}
%define FILELIST_TXT	%{_builddir}/filelist.txt

%description

%prep
%setup -q

%build
@MT_CONF@
make

%install
make DESTDIR=%{buildroot} install

cd %{buildroot}
find -L . -type f | sed 's/^.//' > %{FILELIST_TXT}

%files -f %{FILELIST_TXT}

%post
/sbin/ldconfig

%postun
/sbin/ldconfig
