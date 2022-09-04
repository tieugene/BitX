Name:		BitX
Version:	0.0.2
Release:	1%{?dist}
License:	GPLv3
Summary:	BCE addon
URL:		https://github.com/tieugene/BitX
Source:		https://github.com/tieugene/BitX/archive/%{version}.tar.gz/%{name}-%{version}.tar.gz
BuildRequires:	gcc
BuildRequires:	cmake


%description
Misc bce addons:
- BitBalance: ?
- BitDay: ?
- BitGraph: plot graphs


%prep
%autosetup


%build
%{cmake}
%{cmake_build}


%install
%{cmake_install}


%files
%license LICENSE*
%doc README.md
%{_bindir}/BitBalance
%{_bindir}/BitDay
%{_bindir}/BitGraph


%changelog
* Sun Sep 04 2022 TI_Eugene <ti.eugene@gmail.com> 0.0.2-1
- Version bump

* Tue Aug 16 2022 TI_Eugene <ti.eugene@gmail.com> 0.0.1-1
- Initial packaging
