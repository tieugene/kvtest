Name:		kvtest
Version:	0.0.1
Release:	1%{?dist}
License:	GPLv3
Summary:	Key-value storages tester
URL:		https://github.com/tieugene/%{name}
Source:		https://github.com/tieugene/%{name}/archive/refs/tags/%{version}.tar.gz/%{name}-%{version}.tar.gz
BuildRequires:	gcc-c++
BuildRequires:	cmake
# BerkeleyDB
BuildRequires:	libdb-cxx-devel
# leveldb-devel
BuildRequires:	cmake(leveldb)
# rocksdb-devel
BuildRequires:	pkgconfig(rocksdb)
# lmdb-devel
BuildRequires:	pkgconfig(lmdb)
# kyotocabinet-devel
BuildRequires:	pkgconfig(kyotocabinet)
# tkrzw-devel
BuildRequires:	pkgconfig(tkrzw) >= 0.9.9


%description
Set of tools to test different file-based key-value storages.


%prep
%autosetup


%build
%{cmake}
%{cmake_build}


%install
%{cmake_install}


%files
%license LICENSE
%doc README.md
%{_bindir}/%{name}_*


%changelog
* Sun May 02 2021 TI_Eugene <ti.eugene@gmail.com> 0.0.1-1
- Initial packaging
