CC=gcc
FLAGS=-std=gnu99 -Wall
FILES=eval.c parse.c scamval.c stream.c tokenize.c builtins.c collector.c
TEST_FILES=tests/stream_tests.c tests/tokenize_tests.c tests/parse_tests.c
VALGRIND_FLAGS=-q --leak-check=full --num-callers=500

all: *.c *.h
	$(CC) scam.c $(FILES) -o scam $(FLAGS)

debug: *.c *.h
	$(CC) scam.c $(FILES) -o scam $(FLAGS) -g

test: *.c *.h tests/*.c tests/*.h
	$(CC) tests/tests.c $(FILES) $(TEST_FILES) -o tests/tests $(FLAGS) -g
	$(CC) tests/test_repl.c $(FILES) -o tests/test_repl $(FLAGS) -g
	$(CC) $(FILES) tests/run_test_script.c -o tests/run_test_script $(FLAGS)
	valgrind $(VALGRIND_FLAGS) ./tests/tests
	valgrind $(VALGRIND_FLAGS) ./tests/run_test_script tests/test_stdlib.scm
	valgrind $(VALGRIND_FLAGS) ./tests/run_test_script tests/test_core.scm
	valgrind $(VALGRIND_FLAGS) ./tests/run_test_script tests/test_escapes.scm
