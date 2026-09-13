Name:           meridian-terminal
Version:        2.5.0
Release:        1%{?dist}
Summary:        Modern Linux Terminal & Unified Developer Environment

License:        GPL-3.0-or-later
URL:            https://github.com/charanbalaji2005/Meridian-Shell
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++ >= 10
BuildRequires:  make

Provides:       meridian-shell = %{version}-%{release}
Provides:       meridian = %{version}-%{release}

%description
Meridian Terminal is a fast Linux terminal emulator, developer environment,
and AI agent platform with native TrueColor graphics, live Git intelligence,
multi-pane multiplexing, and in-terminal command preview.

%prep
%setup -q

%build
%make_build all

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{_bindir}
mkdir -p $RPM_BUILD_ROOT%{_datadir}/applications
mkdir -p $RPM_BUILD_ROOT%{_datadir}/meridian/images/gallery
install -m 755 build/meridian-shell $RPM_BUILD_ROOT%{_bindir}/meridian-shell
install -m 755 build/meridian $RPM_BUILD_ROOT%{_bindir}/meridian
install -m 644 meridian.desktop $RPM_BUILD_ROOT%{_datadir}/applications/meridian.desktop
cp -rf resources/images/gallery/* $RPM_BUILD_ROOT%{_datadir}/meridian/images/gallery/

%files
%license LICENSE
%doc README.md
%{_bindir}/meridian-shell
%{_bindir}/meridian
%{_datadir}/applications/meridian.desktop
%{_datadir}/meridian

%changelog
* Sat Aug 29 2026 Charan Balaji <charanbalaji@fedora> - 2.5.0-1
- Release Meridian 2.5 with prebuilt multi-distro installer, VS Code auto-detection, and stats/update engine.
* Tue Aug 25 2026 Charan Balaji <charanbalaji@fedora> - 2.0.0-1
- Release Meridian 2.0 with TrueColor graphics, live Git badges, and unified canvas.
