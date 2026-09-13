# vimguide

[![Build](https://github.com/spxd6666/vimguide/actions/workflows/build.yml/badge.svg)](https://github.com/spxd6666/vimguide/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Release](https://img.shields.io/github/v/release/spxd6666/vimguide)](https://github.com/OWNER/vimguide/releases)


An interactive, full-screen terminal reference for Vim/Vi commands. It shows
a browsable, tabbed table of keybindings grouped by mode/category:

`MOVEMENT · EDITING · YANK/PASTE · SEARCH · INSERT · VISUAL · CMDLINE · REPLACE · WINDOWS · MACROS`

It is written in plain C (C11) using only the standard library, POSIX
termios, and ANSI escape sequences — **no ncurses, no runtime dependencies**
beyond glibc. That keeps compilation and packaging simple across
distributions.

## Key bindings inside the app

| Key                          | Action                              |
|-------------------------------|--------------------------------------|
| `Tab` / `]` / `l` / `Right`   | Next category tab                    |
| `Shift-Tab` / `[` / `h` / `Left` | Previous category tab             |
| `j` / `Down`, `k` / `Up`      | Scroll one row                       |
| `Ctrl-F`/`PageDown`, `Ctrl-B`/`PageUp` | Scroll one page             |
| `g`/`Home`, `G`/`End`         | Jump to top / bottom of current tab  |
| `/`                           | Live search filter within the tab    |
| `Esc`                         | Clear the filter                     |
| `q` / `Ctrl-C`                | Quit                                 |

## Quick install (no build tools required)

Grab the latest `.deb` or `.rpm` from the [Releases](../../releases) page
and install it directly:

```bash
sudo dpkg -i vimguide_*.deb        # Debian/Ubuntu/Mint/etc.
# or
sudo rpm -i vimguide-*.rpm         # Fedora/RHEL/openSUSE/etc.
```

## CLI flags

```bash
vimguide             # open the interactive guide
vimguide --version   # print the version and exit
vimguide --help      # print usage and exit
```

## Building from source

```bash
make            # produces ./vimguide
./vimguide      # run it directly
sudo make install PREFIX=/usr/local   # optional manual install
```

## Building installable packages

Two helper scripts under `scripts/` produce real, installable packages:

```bash
./scripts/build-deb.sh   # -> dist/vimguide_1.0.0_<arch>.deb   (uses dpkg-deb)
./scripts/build-rpm.sh   # -> dist/vimguide-1.0.0-1.<arch>.rpm (uses rpmbuild)
./build.sh               # runs both (skips rpm step if rpmbuild is absent)
```

`build-deb.sh` only needs `dpkg-deb`, which ships by default on
Debian/Ubuntu-family systems.

`build-rpm.sh` needs `rpmbuild`:
- Fedora / RHEL / openSUSE: `sudo dnf install rpm-build` (or `zypper install rpm-build`)
- Debian / Ubuntu (cross-building an rpm): `sudo apt install rpm`

### Installing the packages

```bash
sudo dpkg -i dist/vimguide_1.0.0_amd64.deb      # Debian/Ubuntu/Mint/etc.
sudo apt -f install                             # only if dependencies are missing

sudo rpm -i dist/vimguide-1.0.0-1.*.rpm         # Fedora/RHEL/openSUSE/etc.
# or: sudo dnf install dist/vimguide-1.0.0-1.*.rpm
```

Once installed, run it from anywhere:

```bash
vimguide
```

## Project layout

```
vimguide/
├── src/                 # main.c (TUI + input), data.c/.h (command tables)
├── man/vimguide.1        # man page
├── Makefile
├── packaging/
│   ├── debian/DEBIAN/control
│   └── rpm/vimguide.spec
├── scripts/
│   ├── build-deb.sh
│   └── build-rpm.sh
└── build.sh              # builds everything
```

## Extending the command tables

All the reference content lives in `src/data.c` as plain arrays of
`{key, description}` pairs grouped into tabs (`g_tabs[]`). Add or edit
entries there — no changes to `main.c` are needed to add new rows, and a
new tab is picked up automatically just by adding an entry to `g_tabs[]`.
