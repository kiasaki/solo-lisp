build:
	cc -std=c99 -Wall repl.c mpc.c -ledit -lm -o repl

clean:
	rm repl
