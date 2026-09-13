# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2026-09-12

### Added
- Initial release: full-screen terminal reference for Vim/Vi commands.
- Ten browsable category tabs: Movement, Editing, Yank/Paste, Search,
  Insert, Visual, Command-line, Replace, Windows, Macros.
- Keyboard navigation: tab switching, scrolling, jump to top/bottom,
  live search filter.
- Dependency-free implementation (termios + ANSI escapes, no ncurses).
- `.deb` and `.rpm` packaging scripts, man page, GitHub Actions CI and
  release automation.
