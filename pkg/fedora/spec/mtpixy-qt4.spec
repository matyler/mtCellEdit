Name:			mtpixy-qt4
Version:		@MT_VERSION@
Release:		@MT_RELVER@%{?dist}
Summary:		Qt4 Pixel GUI
License:		GPLv3+
Source:			%{name}-%{version}.tar.gz
BuildRequires:		libmtkit libmtpixy libmtqex4 qt-devel inkscape
Requires:		libmtkit libmtpixy libmtqex4 qt

%global debug_package %{nil}
%define FILELIST_TXT	%{_builddir}/filelist.txt

%description

%prep
%setup -q

%build
@MT_CONF@ --use-qt4
make

%install
make DESTDIR=%{buildroot} install

cd %{buildroot}
find -L . -type f | sed 's/^.//' > %{FILELIST_TXT}

%files -f %{FILELIST_TXT}

%post
  if [ -x usr/bin/update-mime-database ]; then
    echo ">>> Updating Shared MIME-Info database..."
    update-mime-database /usr/share/mime
#    update-mime-database /usr/share/mime > /dev/null 2>&1
  fi

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
  if [ -x usr/bin/update-mime-database ]; then
    echo ">>> Updating Shared MIME-Info database..."
    update-mime-database /usr/share/mime
#    update-mime-database /usr/share/mime > /dev/null 2>&1
  fi

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
