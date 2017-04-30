CC = gcc
LEX = flex
YACC = bison
OBJS = build/builtins.o build/collector.o build/eval.o build/scamval.o build/grammar.o build/flex.o
EXECS = scam tests run_test_script test_repl
DEBUG = -g
PROFILE = -pg
CFLAGS = -Wall $(DEBUG) -std=gnu99 -c -Iinclude
LFLAGS = -Wall $(DEBUG) -lm -lfl -lreadline

all: $(EXECS)

scam: build/scam.o $(OBJS)
	$(CC) $(OBJS) build/scam.o -o scam $(LFLAGS) 

build/builtins.o: src/builtins.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/builtins.c -o build/builtins.o

build/collector.o: src/collector.c include/collector.h
	$(CC) $(CFLAGS) src/collector.c -o build/collector.o

build/eval.o: src/eval.c include/parse.h include/eval.h include/collector.h
	$(CC) $(CFLAGS) src/eval.c -o build/eval.o

build/scam.o: src/scam.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/scam.c -o build/scam.o

build/scamval.o: src/scamval.c src/type.def include/collector.h include/scamval.h
	$(CC) $(CFLAGS) src/scamval.c -o build/scamval.o

build/grammar.o: src/grammar.c src/flex.c
	$(CC) -g -c src/grammar.c -o build/grammar.o -Iinclude

src/grammar.c: src/grammar.y
	$(YACC) -d src/grammar.y
	mv grammar.c src/
	mv grammar.h include/

build/flex.o: src/flex.c
	$(CC) -g -c src/flex.c -o build/flex.o -Iinclude

src/flex.c: src/grammar.l
	$(LEX) src/grammar.l
	mv flex.c src/
	mv flex.h include/

tests: build/tests.o $(OBJS)
	$(CC) $(OBJS) build/tests.o -o tests $(LFLAGS) 

build/tests.o: src/tests.c include/tests.h include/collector.h
	$(CC) $(CFLAGS) src/tests.c -o build/tests.o

run_test_script: build/run_test_script.o $(OBJS)
	$(CC) $(OBJS) build/run_test_script.o -o run_test_script $(LFLAGS) 

build/run_test_script.o: src/run_test_script.c include/collector.h include/eval.h include/scamval.h
	$(CC) $(CFLAGS) src/run_test_script.c -o build/run_test_script.o

test_repl: build/test_repl.o $(OBJS)
	$(CC) $(OBJS) build/test_repl.o -o test_repl $(LFLAGS)

build/test_repl.o: src/test_repl.c include/collector.h include/eval.h include/parse.h 
	$(CC) $(CFLAGS) src/test_repl.c -o build/test_repl.o

clean:
	rm build/*.o src/flex.c src/grammar.c include/grammar.h $(EXECS)

include/collector.h: include/scamval.h

include/eval.h: include/scamval.h

include/scamval.h: src/type.def
