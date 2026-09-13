Name:           vimguide
Version:        1.0.0
Release:        1%{?dist}
Summary:        Interactive full-screen Vim/Vi command reference for the terminal

License:        MIT
URL:            https://example.com/vimguide
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc, make
Requires:       glibc

%description
vimguide is a lightweight, dependency-free terminal application that
shows a browsable, tabbed table of Vim/Vi keybindings grouped by mode:
Movement, Editing, Yank/Paste, Search, Insert, Visual, Command-line,
Replace, Windows, and Macros. Useful as a learning aid while training
muscle memory for modal editing.

%prep
%autosetup -n %{name}-%{version}

%build
%make_build

%install
%make_install PREFIX=/usr

%files
%license LICENSE
%doc README.md
%{_bindir}/vimguide
%{_mandir}/man1/vimguide.1*

%changelog
* Sat Sep 12 2026 Local Builder <you@example.com> - 1.0.0-1
- Initial package
