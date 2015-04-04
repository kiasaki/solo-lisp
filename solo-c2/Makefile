CC = gcc -std=c99 -Wall -g -lm -ledit -Ilib
CORE = core.c readline.c read.c print.c env.c lib/mpc.c

all:
	@mkdir -p build
	$(CC) $(CORE) step0_repl.c -o build/step0_repl
	$(CC) $(CORE) step1_read_print.c -o build/step1_read_print
	$(CC) $(CORE) step2_eval.c -o build/step2_eval
	#$(CC) $(CORE) step0_repl.c -o build/step3_env
	#$(CC) $(CORE) step0_repl.c -o build/step4_if_fn_do
	#$(CC) $(CORE) step0_repl.c -o build/step5_tco
	#$(CC) $(CORE) step0_repl.c -o build/step6_file
	#$(CC) $(CORE) step0_repl.c -o build/step7_quote
	#$(CC) $(CORE) step0_repl.c -o build/step8_macros
	#$(CC) $(CORE) step0_repl.c -o build/step9_try
	#$(CC) $(CORE) step0_repl.c -o build/stepA_mal
