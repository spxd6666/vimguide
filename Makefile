CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11
PREFIX  ?= /usr
BINDIR  := $(DESTDIR)$(PREFIX)/bin
MANDIR  := $(DESTDIR)$(PREFIX)/share/man/man1

SRC := src/main.c src/data.c
BIN := vimguide

.PHONY: all clean install uninstall

all: $(BIN)

$(BIN): $(SRC) src/data.h
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

install: $(BIN)
	install -d $(BINDIR)
	install -m 0755 $(BIN) $(BINDIR)/$(BIN)
	install -d $(MANDIR)
	install -m 0644 man/vimguide.1 $(MANDIR)/vimguide.1

uninstall:
	rm -f $(BINDIR)/$(BIN)
	rm -f $(MANDIR)/vimguide.1

clean:
	rm -f $(BIN)
