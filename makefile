CC=gcc
FLAGS=-std=gnu99 -Wall
FILES=eval.c parse.c scamval.c stream.c tokenize.c builtins.c progutils.c collector.c
TEST_FILES=test_files/stream_tests.c test_files/tokenize_tests.c test_files/parse_tests.c test_files/eval_tests.c
VALGRIND_FLAGS=-q --leak-check=full --num-callers=500

all: *.c *.h
	$(CC) scam.c $(FILES) -o scam $(FLAGS)

debug: *.c *.h
	$(CC) scam.c $(FILES) -o scam $(FLAGS) -g

tests: *.c *.h test_files/*.c test_files/*.h
	$(CC) test_files/tests.c $(FILES) $(TEST_FILES) -o test_files/tests $(FLAGS) -g
	$(CC) test_files/test_repl.c $(FILES) -o test_files/test_repl $(FLAGS) -g
	$(CC) $(FILES) test_files/test_eq.c -o test_files/test_eq $(FLAGS)
	$(CC) $(FILES) test_files/test_err.c -o test_files/test_err $(FLAGS)
	valgrind $(VALGRIND_FLAGS) ./test_files/tests
	./test_files/run_tests.py
