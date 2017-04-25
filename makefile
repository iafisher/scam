CC = gcc
OBJS = lib/builtins.o lib/collector.o lib/eval.o lib/parse.o lib/scamval.o lib/stream.o lib/tokenize.o
TEST_OBJS = lib/parse_tests.o lib/stream_tests.o lib/tokenize_tests.o
DEBUG = -g
CFLAGS = -Wall $(DEBUG) -std=gnu99 -c -Iinclude
LFLAGS = -Wall $(DEBUG) -lm

scam: lib/scam.o $(OBJS)
	$(CC) $(OBJS) lib/scam.o -o scam $(LFLAGS) 

lib/builtins.o: src/builtins.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/builtins.c -o lib/builtins.o

lib/collector.o: src/collector.c include/collector.h
	$(CC) $(CFLAGS) src/collector.c -o lib/collector.o

lib/eval.o: src/eval.c include/parse.h include/eval.h include/collector.h
	$(CC) $(CFLAGS) src/eval.c -o lib/eval.o

lib/parse.o: src/parse.c include/collector.h include/parser.h include/tokenize.h
	$(CC) $(CFLAGS) src/parse.c -o lib/parse.o

lib/scam.o: src/scam.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/scam.c -o lib/scam.o

lib/scamval.o: src/scamval.c include/collector.h include/scamval.h
	$(CC) $(CFLAGS) src/scamval.c -o lib/scamval.o

lib/stream.o: src/stream.c include/stream.h include/collector.h
	$(CC) $(CFLAGS) src/stream.c -o lib/stream.o

lib/tokenize.o: src/tokenize.c include/tokenize.h
	$(CC) $(CFLAGS) src/tokenize.c -o lib/tokenize.o

tests: lib/tests.o $(OBJS) $(TEST_OBJS)
	$(CC) $(OBJS) $(TEST_OBJS) lib/tests.o -o tests $(LFLAGS) 

lib/tests.o: src/tests.c include/tests.h
	$(CC) $(CFLAGS) src/tests.c -o lib/tests.o

lib/parse_tests.o: src/parse_tests.c include/parse.h include/scamval.h
	$(CC) $(CFLAGS) src/parse_tests.c -o lib/parse_tests.o

lib/stream_tests.o: src/stream_tests.c include/stream.h include/tests.h
	$(CC) $(CFLAGS) src/stream_tests.c -o lib/stream_tests.o

lib/tokenize_tests.o: src/tokenize_tests.c include/tokenize.h
	$(CC) $(CFLAGS) src/tokenize_tests.c -o lib/tokenize_tests.o

run_test_script: lib/run_test_script.o $(OBJS)
	$(CC) $(OBJS) lib/run_test_script.o -o run_test_script $(LFLAGS) 

lib/run_test_script.o: src/run_test_script.c include/collector.h include/eval.h include/scamval.h
	$(CC) $(CFLAGS) src/run_test_script.c -o lib/run_test_script.o

test_repl: lib/test_repl.o $(OBJS)
	$(CC) $(OBJS) lib/test_repl.o -o test_repl $(LFLAGS)

lib/test_repl.o: src/test_repl.c include/collector.h include/eval.h include/parse.h include/stream.h include/tokenize.h
	$(CC) $(CFLAGS) src/test_repl.c -o lib/test_repl.o

clean:
	rm lib/*.o

include/parser.h: include/tokenize.h include/scamval.h

include/collector.h: include/scamval.h

include/eval.h: include/scamval.h

include/tokenize.h: include/stream.h
