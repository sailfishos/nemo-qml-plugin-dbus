Name:       nemo-qml-plugin-dbus-qt5
Summary:    DBus plugin for Nemo Mobile
Version:    2.1.16
Release:    1
Group:      System/Libraries
License:    LGPLv2.1
URL:        https://git.merproject.org/mer-core/nemo-qml-plugin-dbus
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  mer-qdoc-template

%description
%{summary}.

%package devel
Summary:    Development libraries for nemo
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(Qt5DBus)

%description devel
%{summary}.

%package tests
Summary:    DBus plugin tests
Group:      System/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtdeclarative-import-qttest
Requires:   qt5-qtdeclarative-devel-tools
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)

%description tests
%{summary}.

%package doc
Summary:    DBus plugin documentation
Group:      System/Libraries

%description doc
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}"
make  %{?_smp_mflags}
make -C tests/dbustestd  %{?_smp_mflags}
make  %{?_smp_mflags} docs

%install
rm -rf %{buildroot}
%qmake5_install
make -C tests/dbustestd install ROOT=%{buildroot} VERS=%{version}
make install_docs INSTALL_ROOT=%{buildroot}

# org.nemomobile.dbus legacy import
mkdir -p %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/dbus/
ln -sf %{_libdir}/qt5/qml/Nemo/DBus/libnemodbus.so %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/dbus/
sed 's/Nemo.DBus/org.nemomobile.dbus/' < src/plugin/qmldir > %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/dbus/qmldir

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%dir %{_libdir}/qt5/qml/Nemo/DBus
%{_libdir}/qt5/qml/Nemo/DBus/libnemodbus.so
%{_libdir}/qt5/qml/Nemo/DBus/qmldir
%{_libdir}/qt5/qml/Nemo/DBus/plugins.qmltypes

# org.nemomobile.dbus legacy import
%dir %{_libdir}/qt5/qml/org/nemomobile/dbus
%{_libdir}/qt5/qml/org/nemomobile/dbus/libnemodbus.so
%{_libdir}/qt5/qml/org/nemomobile/dbus/qmldir

# library
%{_libdir}/libnemodbus.so.*

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/nemo-dbus
%dir %{_includedir}/nemo-dbus/private
%{_includedir}/nemo-dbus/*.h
%{_includedir}/nemo-dbus/private/*.h
%{_libdir}/libnemodbus.so
%{_libdir}/pkgconfig/nemodbus.pc

%files tests
%defattr(-,root,root,-)
%dir /opt/tests/nemo-qml-plugins-qt5/dbus
%dir /usr/share/dbus-1/services
/opt/tests/nemo-qml-plugins-qt5/dbus/*
%{_datadir}/dbus-1/services/org.nemomobile.dbustestd.service

%files doc
%defattr(-,root,root,-)
%dir %{_datadir}/doc/nemo-qml-plugin-dbus
%{_datadir}/doc/nemo-qml-plugin-dbus/nemo-qml-plugin-dbus.qch
