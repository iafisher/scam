CC = gcc
LEX = flex
YACC = bison
OBJS = build/builtins.o build/collector.o build/eval.o build/grammar.o build/flex.o build/scamval/cmp.o build/scamval/dict.o build/scamval/misc.o build/scamval/num.o build/scamval/seq.o build/scamval/str.o
EXECS = scam tests run_test_script benchmark
DEBUG = -g
PROFILE = -pg
CFLAGS = -Wall -Wextra $(DEBUG) -std=gnu99 -c -Iinclude
LFLAGS = -Wall -Wextra $(DEBUG) -lm -lfl -lreadline

all: $(EXECS)

scam: build/scam.o $(OBJS)
	$(CC) $(OBJS) build/scam.o -o scam $(LFLAGS) 

benchmark: build/benchmark.o $(OBJS)
	$(CC) $(OBJS) build/benchmark.o -o benchmark $(LFLAGS) 

build/benchmark.o: src/benchmark.c include/parse.h include/eval.h include/scamval.h include/collector.h
	$(CC) $(CFLAGS) src/benchmark.c -o build/benchmark.o

build/builtins.o: src/builtins.c include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/builtins.c -o build/builtins.o

build/collector.o: src/collector.c include/collector.h
	$(CC) $(CFLAGS) src/collector.c -o build/collector.o

build/eval.o: src/eval.c include/parse.h include/eval.h include/collector.h
	$(CC) $(CFLAGS) src/eval.c -o build/eval.o

build/scam.o: src/scam.c include/collector.h include/eval.h include/parse.h
	$(CC) $(CFLAGS) src/scam.c -o build/scam.o

build/scamval/cmp.o: src/scamval/cmp.c include/scamval.h
	$(CC) $(CFLAGS) src/scamval/cmp.c -o build/scamval/cmp.o

build/scamval/dict.o: src/scamval/dict.c include/scamval.h include/collector.h
	$(CC) $(CFLAGS) src/scamval/dict.c -o build/scamval/dict.o

build/scamval/misc.o: src/scamval/misc.c src/escape.def include/scamval.h include/collector.h src/type.def
	$(CC) $(CFLAGS) src/scamval/misc.c -o build/scamval/misc.o

build/scamval/num.o: src/scamval/num.c include/scamval.h include/collector.h
	$(CC) $(CFLAGS) src/scamval/num.c -o build/scamval/num.o

build/scamval/seq.o: src/scamval/seq.c include/scamval.h include/collector.h
	$(CC) $(CFLAGS) src/scamval/seq.c -o build/scamval/seq.o

build/scamval/str.o: src/scamval/str.c src/escape.def include/scamval.h include/collector.h 
	$(CC) $(CFLAGS) src/scamval/str.c -o build/scamval/str.o

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

build/tests.o: src/tests.c include/tests.h include/collector.h include/eval.h
	$(CC) $(CFLAGS) src/tests.c -o build/tests.o

run_test_script: build/run_test_script.o $(OBJS)
	$(CC) $(OBJS) build/run_test_script.o -o run_test_script $(LFLAGS) 

build/run_test_script.o: src/run_test_script.c include/collector.h include/eval.h include/scamval.h
	$(CC) $(CFLAGS) src/run_test_script.c -o build/run_test_script.o

clean:
	rm -f build/*.o build/scamval/*.o src/flex.c src/grammar.c include/grammar.h $(EXECS)

include/collector.h: include/scamval.h

include/eval.h: include/scamval.h

include/scamval.h: src/type.def
