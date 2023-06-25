Name:			mtnush-qt6
Version:		@MT_VERSION@
Release:		@MT_RELVER@%{?dist}
Summary:		Qt6 Big Number Calculator
License:		GPLv3+
Source:			%{name}-%{version}.tar.gz
BuildRequires:		libmtkit libmtcelledit libmtdatawell libmtqex6 qt6-base-devel inkscape gmp-devel mpfr-devel
Requires:		libmtkit libmtcelledit libmtdatawell libmtqex6 libQt6Core6 libgmp10 libmpfr6

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
  if [ -x usr/bin/xdg-icon-resource ]; then
    echo ">>> Updating desktop icon system..."
    xdg-icon-resource forceupdate --theme hicolor
#    xdg-icon-resource forceupdate --theme hicolor > /dev/null 2>&1
  fi

  if [ -x usr/bin/update-desktop-database ]; then
    echo ">>> Updating MIME-desktop association cache database..."
    update-desktop-database
#    usr/bin/update-desktop-database -q
  fi

%postun
  if [ -x usr/bin/xdg-icon-resource ]; then
    echo ">>> Updating desktop icon system..."
    xdg-icon-resource forceupdate --theme hicolor
#    xdg-icon-resource forceupdate --theme hicolor > /dev/null 2>&1
  fi

  if [ -x usr/bin/update-desktop-database ]; then
    echo ">>> Updating MIME-desktop association cache database..."
    update-desktop-database
#    usr/bin/update-desktop-database -q
  fi
