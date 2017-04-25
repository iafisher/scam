#!/bin/bash

valgrind -q --leak-check=full --num-callers=500 ./tests/tests
valgrind -q --leak-check=full --num-callers=500 ./tests/run_test_script tests/test_stdlib.scm
valgrind -q --leak-check=full --num-callers=500 ./tests/run_test_script tests/test_core.scm
valgrind -q --leak-check=full --num-callers=500 ./tests/run_test_script tests/test_escapes.scm
valgrind -q --leak-check=full --num-callers=500 ./tests/run_test_script tests/test_dict.scm
