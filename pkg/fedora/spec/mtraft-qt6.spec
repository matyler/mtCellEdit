Name:			mtraft-qt6
Version:		@MT_VERSION@
Release:		@MT_RELVER@%{?dist}
Summary:		Qt6 Directory Analysis Tool
License:		GPLv3+
Source:			%{name}-%{version}.tar.gz
BuildRequires:		libmtkit libmtcelledit libmtqex6 qt6-qtbase-devel inkscape
Requires:		libmtkit libmtcelledit libmtqex6 qt6-qtbase

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
