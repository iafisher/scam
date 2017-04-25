CC = gcc
OBJS = builtins.o collector.o eval.o parse.o scamval.o stream.o tokenize.o
TEST_OBJS = tests/parse_tests.o tests/stream_tests.o tests/tokenize_tests.o
DEBUG = -g
CFLAGS = -Wall $(DEBUG) -std=gnu99 -c
LFLAGS = -Wall $(DEBUG) -lm

scam: scam.o $(OBJS)
	$(CC) $(OBJS) scam.o -o scam $(LFLAGS) 

builtins.o: builtins.c collector.h eval.h
	$(CC) $(CFLAGS) builtins.c

collector.o: collector.c collector.h
	$(CC) $(CFLAGS) collector.c

eval.o: eval.c parse.h eval.h collector.h
	$(CC) $(CFLAGS) eval.c

parse.o: parse.c collector.h parser.h tokenize.h
	$(CC) $(CFLAGS) parse.c

scam.o: scam.c collector.h eval.h
	$(CC) $(CFLAGS) scam.c

scamval.o: scamval.c collector.h scamval.h
	$(CC) $(CFLAGS) scamval.c

stream.o: stream.c stream.h collector.h
	$(CC) $(CFLAGS) stream.c

tokenize.o: tokenize.c tokenize.h
	$(CC) $(CFLAGS) tokenize.c

tests/tests: tests/tests.o $(OBJS) $(TEST_OBJS)
	$(CC) $(OBJS) $(TEST_OBJS) tests/tests.o -o tests/tests $(LFLAGS) 

tests/tests.o: tests/tests.c tests/tests.h
	$(CC) $(CFLAGS) tests/tests.c -o tests/tests.o

tests/parse_tests.o: tests/parse_tests.c parse.h scamval.h
	$(CC) $(CFLAGS) tests/parse_tests.c -o tests/parse_tests.o

tests/stream_tests.o: tests/stream_tests.c stream.h tests/tests.h
	$(CC) $(CFLAGS) tests/stream_tests.c -o tests/stream_tests.o

tests/tokenize_tests.o: tests/tokenize_tests.c tokenize.h
	$(CC) $(CFLAGS) tests/tokenize_tests.c -o tests/tokenize_tests.o

tests/run_test_script: tests/run_test_script.o $(OBJS)
	$(CC) $(OBJS) tests/run_test_script.o -o tests/run_test_script $(LFLAGS) 

tests/run_test_script.o: tests/run_test_script.c collector.h eval.h scamval.h
	$(CC) $(CFLAGS) tests/run_test_script.c -o tests/run_test_script.o

clean:
	rm *.o

parser.h: tokenize.h scamval.h

collector.h: scamval.h

eval.h: scamval.h

tokenize.h: stream.h
