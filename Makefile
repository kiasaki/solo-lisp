.PHONY: all build debug clean

all: build

build:
	cc -std=c99 -Wall repl.c mpc.c lang.c -ledit -lm -o repl

debug: build
	cc -std=c99 -g -ggdb -Wall repl.c mpc.c lang.c -ledit -lm -o repl
	lldb repl

clean:
	rm repl
	rm -r repl.dSYM/
