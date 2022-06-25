Name:			libmtqex6
Version:		@MT_VERSION@
Release:		@MT_RELVER@%{?dist}
Summary:		Qt6 Dialogs and Utility Functions
License:		GPLv3+
Source:			%{name}-%{version}.tar.gz
BuildRequires:		libmtkit libmtpixy qt6-qtbase-devel
Requires:		libmtkit libmtpixy qt6-qtbase

%global debug_package %{nil}
%define FILELIST_TXT	%{_builddir}/filelist.txt

%description

%prep
%setup -q

%build
@MT_CONF@ --use-qt6
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
