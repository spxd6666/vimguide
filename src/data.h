#ifndef VIMGUIDE_DATA_H
#define VIMGUIDE_DATA_H

typedef struct {
    const char *key;    /* the vim keystroke / command */
    const char *desc;   /* what it does */
} VimEntry;

typedef struct {
    const char *title;        /* tab title, e.g. "NORMAL MODE" */
    const char *subtitle;     /* short description of the mode */
    const VimEntry *entries;  /* array of key/desc pairs */
    int count;                /* number of entries */
} VimTab;

/* Exposed to main.c */
extern const VimTab g_tabs[];
extern const int g_tab_count;

#endif
