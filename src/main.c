/*
 * vimguide - an interactive full-screen terminal reference for Vim/Vi
 * commands, organized by mode/category into browsable tabs.
 *
 * Deliberately dependency-free: uses only termios + ANSI escape codes,
 * so it needs nothing beyond a standard C library and a terminfo-capable
 * terminal emulator. No ncurses, no external libraries.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdarg.h>

#include "data.h"

#define VERSION "1.0.0"

static struct termios g_orig_termios;
static int g_rows = 24, g_cols = 80;
static volatile sig_atomic_t g_resized = 0;

/* ---------------- terminal handling ---------------- */

static void die(const char *msg) {
    /* best-effort restore before bailing */
    write(STDOUT_FILENO, "\x1b[?25h\x1b[?1049l", 14);
    fprintf(stderr, "vimguide: %s\n", msg);
    exit(1);
}

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
    /* leave alternate screen, show cursor */
    write(STDOUT_FILENO, "\x1b[?25h\x1b[?1049l", 14);
}

static void enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) == -1) die("tcgetattr failed");
    atexit(disable_raw_mode);

    struct termios raw = g_orig_termios;
    raw.c_iflag &= ~(unsigned long)(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(unsigned long)(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(unsigned long)(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; /* 100ms read timeout, lets us poll for resize */

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr failed");

    /* enter alternate screen, hide cursor */
    write(STDOUT_FILENO, "\x1b[?1049h\x1b[?25l", 14);
}

static void get_win_size(void) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        g_cols = ws.ws_col;
        g_rows = ws.ws_row;
    }
}

static void on_winch(int sig) {
    (void)sig;
    g_resized = 1;
}

/* ---------------- small output helpers ---------------- */

static void out(const char *s) { write(STDOUT_FILENO, s, strlen(s)); }

static void outf(const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    out(buf);
}

/* colors kept to the portable 8/16-color ANSI set for maximum compatibility */
#define C_RESET   "\x1b[0m"
#define C_BOLD    "\x1b[1m"
#define C_DIM     "\x1b[2m"
#define C_REV     "\x1b[7m"
#define C_FG_CYAN   "\x1b[36m"
#define C_FG_GREEN  "\x1b[32m"
#define C_FG_YELLOW "\x1b[33m"
#define C_FG_WHITE  "\x1b[37m"
#define C_FG_BLACK  "\x1b[30m"
#define C_BG_CYAN   "\x1b[46m"
#define C_BG_BLUE   "\x1b[44m"

/* ---------------- UTF-8 box drawing (widely supported) ---------------- */
#define BOX_H  "\xe2\x94\x80" /* ─ */
#define BOX_V  "\xe2\x94\x82" /* │ */
#define BOX_TL "\xe2\x94\x8c" /* ┌ */
#define BOX_TR "\xe2\x94\x90" /* ┐ */
#define BOX_BL "\xe2\x94\x94" /* └ */
#define BOX_BR "\xe2\x94\x98" /* ┘ */
#define BOX_LT "\xe2\x94\x9c" /* ├ */
#define BOX_RT "\xe2\x94\xa4" /* ┤ */

static void repeat_str(const char *s, int n) {
    for (int i = 0; i < n; i++) out(s);
}

/* ---------------- application state ---------------- */

typedef struct {
    int cur_tab;
    int scroll;      /* first visible row index within current tab */
    char filter[64]; /* live search filter, empty = none */
    int filtering;   /* 1 while typing a filter */
} AppState;

/* returns 1 if entry matches the current filter (case-insensitive substring
 * match against key or description), always 1 if filter is empty */
static int entry_matches(const VimEntry *e, const char *filter) {
    if (!filter || filter[0] == '\0') return 1;
    char hay[256];
    snprintf(hay, sizeof(hay), "%s %s", e->key, e->desc);
    char needle[64];
    size_t i;
    for (i = 0; filter[i] && i < sizeof(needle) - 1; i++)
        needle[i] = (char)tolower((unsigned char)filter[i]);
    needle[i] = '\0';
    for (char *p = hay; *p; p++) *p = (char)tolower((unsigned char)*p);
    return strstr(hay, needle) != NULL;
}

static void draw(AppState *st) {
    get_win_size();
    char seq[32];
    snprintf(seq, sizeof(seq), "\x1b[H");
    out(seq); /* cursor home */
    out("\x1b[2J");

    int width = g_cols < 40 ? 40 : (g_cols > 200 ? 200 : g_cols);

    /* ---- title bar ---- */
    outf(C_BG_BLUE C_FG_WHITE C_BOLD " vimguide v%s " C_RESET C_DIM
         "  interactive Vim / Vi command reference" C_RESET "\r\n", VERSION);

    /* ---- tab bar ---- */
    for (int i = 0; i < g_tab_count; i++) {
        if (i == st->cur_tab) {
            outf(C_REV C_BOLD " %s " C_RESET, g_tabs[i].title);
        } else {
            outf(C_DIM " %s " C_RESET, g_tabs[i].title);
        }
    }
    out("\r\n");

    const VimTab *tab = &g_tabs[st->cur_tab];
    outf(C_FG_YELLOW "%s" C_RESET "\r\n", tab->subtitle);

    /* ---- build filtered index list ---- */
    int idxs[512];
    int n = 0;
    for (int i = 0; i < tab->count && n < 512; i++) {
        if (entry_matches(&tab->entries[i], st->filter)) idxs[n++] = i;
    }

    /* header row + border */
    int key_w = width < 70 ? 16 : 22;
    int desc_w = width - key_w - 5;
    if (desc_w < 10) desc_w = 10;

    out(C_FG_CYAN);
    out(BOX_TL); repeat_str(BOX_H, key_w + 2); out(BOX_H);
    out(BOX_H); repeat_str(BOX_H, desc_w + 1); out(BOX_TR); out(C_RESET "\r\n");

    outf(C_FG_CYAN BOX_V C_RESET C_BOLD " %-*s " C_FG_CYAN BOX_V C_RESET C_BOLD " %-*s" C_FG_CYAN " " BOX_V C_RESET "\r\n",
         key_w, "KEY / COMMAND", desc_w, "ACTION");

    out(C_FG_CYAN);
    out(BOX_LT); repeat_str(BOX_H, key_w + 2); out(BOX_H);
    out(BOX_H); repeat_str(BOX_H, desc_w + 1); out(BOX_RT); out(C_RESET "\r\n");

    /* how many rows fit: total rows minus title(1) tabbar(1) subtitle(1)
     * top-border(1) header(1) header-sep(1) bottom-border(1) footer(2) */
    int reserved = 9;
    int visible_rows = g_rows - reserved;
    if (visible_rows < 1) visible_rows = 1;

    if (st->scroll > n - visible_rows) st->scroll = n - visible_rows;
    if (st->scroll < 0) st->scroll = 0;

    for (int r = 0; r < visible_rows; r++) {
        int i = st->scroll + r;
        if (i >= n) {
            outf(C_FG_CYAN BOX_V C_RESET " %-*s " C_FG_CYAN BOX_V C_RESET " %-*s " C_FG_CYAN BOX_V C_RESET "\r\n",
                 key_w, "", desc_w, "");
            continue;
        }
        const VimEntry *e = &tab->entries[idxs[i]];
        char keybuf[64]; snprintf(keybuf, sizeof(keybuf), "%.*s", key_w, e->key);
        char descbuf[256]; snprintf(descbuf, sizeof(descbuf), "%.*s", desc_w, e->desc);
        outf(C_FG_CYAN BOX_V C_RESET C_FG_GREEN C_BOLD " %-*s " C_RESET
             C_FG_CYAN BOX_V C_RESET " %-*s " C_FG_CYAN BOX_V C_RESET "\r\n",
             key_w, keybuf, desc_w, descbuf);
    }

    out(C_FG_CYAN);
    out(BOX_BL); repeat_str(BOX_H, key_w + 2); out(BOX_H);
    out(BOX_H); repeat_str(BOX_H, desc_w + 1); out(BOX_BR); out(C_RESET "\r\n");

    /* ---- footer / status ---- */
    if (st->filtering) {
        outf(C_FG_YELLOW "Search: %s" C_RESET "%s\r\n", st->filter, "_");
    } else if (st->filter[0]) {
        outf(C_DIM "Filter: \"%s\" (%d match%s) - press " C_RESET C_BOLD "Esc" C_RESET C_DIM
             " to clear" C_RESET "\r\n", st->filter, n, n == 1 ? "" : "es");
    } else {
        outf(C_DIM "%d commands in this tab" C_RESET "\r\n", n);
    }
    out(C_DIM "Tab/Shift-Tab or " C_RESET C_BOLD "[ ]" C_RESET C_DIM
        " switch category  |  j/k or arrows scroll  |  g/G top/bottom  |  / search  |  q quit" C_RESET "\r\n");

    fflush(stdout);
}

/* ---------------- input handling ---------------- */

/* returns 0 on timeout/no-key, otherwise a code:
 *  normal ASCII char, or one of the KEY_* pseudo-codes below */
enum { KEY_NONE = 0, KEY_UP = 1000, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
       KEY_PGUP, KEY_PGDN, KEY_HOME, KEY_END, KEY_BACKTAB };

static int read_key(void) {
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return KEY_NONE;

    if (c == '\x1b') {
        unsigned char seq[4];
        if (read(STDIN_FILENO, &seq[0], 1) <= 0) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) <= 0) return '\x1b';
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) <= 0) return KEY_NONE;
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '5': return KEY_PGUP;
                        case '6': return KEY_PGDN;
                        case '1': return KEY_HOME;
                        case '4': return KEY_END;
                    }
                }
                return KEY_NONE;
            }
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
                case 'Z': return KEY_BACKTAB;
            }
        }
        return '\x1b';
    }
    return c;
}

static void print_usage(FILE *stream) {
    fprintf(stream,
        "vimguide %s - interactive Vim/Vi command reference\n"
        "\n"
        "Usage: vimguide [--version | --help]\n"
        "\n"
        "Run with no arguments to open the interactive full-screen guide.\n"
        "Inside the app:\n"
        "  Tab / ]      next category tab\n"
        "  Shift-Tab/[  previous category tab\n"
        "  j/k          scroll\n"
        "  g/G          top / bottom of tab\n"
        "  /            search filter, Esc to clear\n"
        "  q            quit\n",
        VERSION);
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("vimguide %s\n", VERSION);
            return 0;
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(stdout);
            return 0;
        }
        fprintf(stderr, "vimguide: unknown option '%s'\n", argv[i]);
        print_usage(stderr);
        return 1;
    }

    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        fprintf(stderr, "vimguide: this program needs an interactive terminal.\n");
        return 1;
    }

    signal(SIGWINCH, on_winch);
    enable_raw_mode();
    get_win_size();

    AppState st;
    memset(&st, 0, sizeof(st));

    draw(&st);

    int running = 1;
    while (running) {
        if (g_resized) { g_resized = 0; draw(&st); }

        int k = read_key();
        if (k == KEY_NONE) continue;

        int need_redraw = 1;
        const VimTab *tab = &g_tabs[st.cur_tab];

        if (st.filtering) {
            if (k == 27 /* Esc */) {
                st.filtering = 0;
            } else if (k == '\r' || k == '\n') {
                st.filtering = 0;
            } else if (k == 127 || k == 8 /* backspace */) {
                size_t len = strlen(st.filter);
                if (len > 0) st.filter[len - 1] = '\0';
            } else if (k >= 32 && k < 127) {
                size_t len = strlen(st.filter);
                if (len < sizeof(st.filter) - 1) {
                    st.filter[len] = (char)k;
                    st.filter[len + 1] = '\0';
                }
            }
            draw(&st);
            continue;
        }

        switch (k) {
            case 'q': case 'Q': case 3 /* Ctrl-C */:
                running = 0;
                break;
            case '\t': case KEY_RIGHT: case 'l': case ']':
                st.cur_tab = (st.cur_tab + 1) % g_tab_count;
                st.scroll = 0;
                st.filter[0] = '\0';
                break;
            case KEY_BACKTAB: case KEY_LEFT: case 'h': case '[':
                st.cur_tab = (st.cur_tab - 1 + g_tab_count) % g_tab_count;
                st.scroll = 0;
                st.filter[0] = '\0';
                break;
            case 'j': case KEY_DOWN:
                st.scroll++;
                break;
            case 'k': case KEY_UP:
                st.scroll--;
                break;
            case KEY_PGDN: case 6 /* Ctrl-F */:
                st.scroll += (g_rows - 9 > 1 ? g_rows - 9 : 1);
                break;
            case KEY_PGUP: case 2 /* Ctrl-B */:
                st.scroll -= (g_rows - 9 > 1 ? g_rows - 9 : 1);
                break;
            case 'g': case KEY_HOME:
                st.scroll = 0;
                break;
            case 'G': case KEY_END:
                st.scroll = tab->count;
                break;
            case '/':
                st.filtering = 1;
                st.filter[0] = '\0';
                break;
            case 27: /* Esc clears filter */
                st.filter[0] = '\0';
                st.scroll = 0;
                break;
            default:
                need_redraw = 0;
                break;
        }
        if (st.scroll < 0) st.scroll = 0;
        if (need_redraw) draw(&st);
    }

    return 0;
}
