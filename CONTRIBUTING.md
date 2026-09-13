# Contributing to vimguide

Thanks for considering a contribution! This project intentionally stays
small and dependency-free, so contributions that keep it that way are
easiest to merge.

## Development setup

```bash
git clone https://github.com/<your-username>/vimguide.git
cd vimguide
make
./vimguide
```

No dependencies beyond a C11 compiler and a POSIX libc (termios) are
required — please keep it that way. Do not introduce ncurses or other
third-party libraries without discussion in an issue first.

## Adding or correcting Vim commands

All reference content lives in `src/data.c` as plain
`{key, description}` arrays grouped into tabs (see `g_tabs[]` at the
bottom of the file). To add a command:

1. Find the relevant tab array (e.g. `tab_move`, `tab_edit`, `tab_visual`...).
2. Add a `{"key", "description"}` line, following the style of nearby entries
   (short, imperative descriptions, consistent capitalization).
3. Rebuild and eyeball it: `make && ./vimguide`.

To add a whole new tab, add a new `static const VimEntry[]` array and
register it in the `g_tabs[]` list at the bottom of `src/data.c`.

## Code style

- C11, compiled with `-Wall -Wextra`; please keep the build warning-free
  for anything you touch.
- No ncurses / external TUI libraries — the app deliberately uses only
  termios + ANSI escapes so it has zero runtime dependencies.
- Keep `main.c` (rendering/input) and `data.c` (content) separated.

## Testing your changes

There's no interactive test harness beyond running the app, but CI runs
a headless smoke test over a pty on every push/PR (see
`.github/workflows/build.yml`). Before opening a PR:

```bash
make                      # must build without new warnings
./vimguide                # sanity-check visually
./scripts/build-deb.sh    # if you touched packaging
```

## Submitting changes

1. Fork the repo and create a branch off `main`.
2. Make your change, keeping commits focused.
3. Open a pull request describing what changed and why.

## Reporting bugs / requesting features

Please use the issue templates provided when opening a new issue.
