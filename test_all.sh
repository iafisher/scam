#!/bin/bash

valgrind -q --leak-check=full --num-callers=500 ./tests
valgrind -q --leak-check=full --num-callers=500 ./run_test_script resources/test_stdlib.scm
valgrind -q --leak-check=full --num-callers=500 ./run_test_script resources/test_core.scm
valgrind -q --leak-check=full --num-callers=500 ./run_test_script resources/test_escapes.scm
valgrind -q --leak-check=full --num-callers=500 ./run_test_script resources/test_dict.scm
