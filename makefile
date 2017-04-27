CC = gcc
OBJS = build/builtins.o build/collector.o build/eval.o build/parse.o build/scamval.o build/stream.o build/tokenize.o
TEST_OBJS = build/parse_tests.o build/stream_tests.o build/tokenize_tests.o
DEBUG = -g
PROFILE = -pg
CFLAGS = -Wall $(DEBUG) -std=gnu99 -c -Iinclude
LFLAGS = -Wall $(DEBUG) -lm

all: scam tests run_test_script test_repl

scam: build/scam.o $(OBJS)
	$(CC) $(OBJS) build/scam.o -o scam $(LFLAGS) 

build/builtins.o: src/builtins.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/builtins.c -o build/builtins.o

build/collector.o: src/collector.c include/collector.h
	$(CC) $(CFLAGS) src/collector.c -o build/collector.o

build/eval.o: src/eval.c include/parse.h include/eval.h include/collector.h
	$(CC) $(CFLAGS) src/eval.c -o build/eval.o

build/parse.o: src/parse.c include/collector.h include/parser.h include/tokenize.h
	$(CC) $(CFLAGS) src/parse.c -o build/parse.o

build/scam.o: src/scam.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/scam.c -o build/scam.o

build/scamval.o: src/scamval.c include/collector.h include/scamval.h
	$(CC) $(CFLAGS) src/scamval.c -o build/scamval.o

build/stream.o: src/stream.c include/stream.h include/collector.h
	$(CC) $(CFLAGS) src/stream.c -o build/stream.o

build/tokenize.o: src/tokenize.c include/tokenize.h
	$(CC) $(CFLAGS) src/tokenize.c -o build/tokenize.o

tests: build/tests.o $(OBJS) $(TEST_OBJS)
	$(CC) $(OBJS) $(TEST_OBJS) build/tests.o -o tests $(LFLAGS) 

build/tests.o: src/tests.c include/tests.h include/collector.h
	$(CC) $(CFLAGS) src/tests.c -o build/tests.o

build/parse_tests.o: src/parse_tests.c include/parse.h include/scamval.h include/collector.h
	$(CC) $(CFLAGS) src/parse_tests.c -o build/parse_tests.o

build/stream_tests.o: src/stream_tests.c include/stream.h include/tests.h
	$(CC) $(CFLAGS) src/stream_tests.c -o build/stream_tests.o

build/tokenize_tests.o: src/tokenize_tests.c include/tokenize.h
	$(CC) $(CFLAGS) src/tokenize_tests.c -o build/tokenize_tests.o

run_test_script: build/run_test_script.o $(OBJS)
	$(CC) $(OBJS) build/run_test_script.o -o run_test_script $(LFLAGS) 

build/run_test_script.o: src/run_test_script.c include/collector.h include/eval.h include/scamval.h
	$(CC) $(CFLAGS) src/run_test_script.c -o build/run_test_script.o

test_repl: build/test_repl.o $(OBJS)
	$(CC) $(OBJS) build/test_repl.o -o test_repl $(LFLAGS)

build/test_repl.o: src/test_repl.c include/collector.h include/eval.h include/parse.h include/stream.h include/tokenize.h
	$(CC) $(CFLAGS) src/test_repl.c -o build/test_repl.o

clean:
	rm build/*.o scam tests run_test_script test_repl

include/parser.h: include/tokenize.h include/scamval.h

include/collector.h: include/scamval.h

include/eval.h: include/scamval.h

include/tokenize.h: include/stream.h
