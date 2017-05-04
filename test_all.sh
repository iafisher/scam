#!/bin/bash

valgrind -q --leak-check=full --show-leak-kinds=all --num-callers=500 ./tests
echo resources/lib/test_* resources/lang/test_* | xargs -n 3 valgrind -q --leak-check=full --show-leak-kinds=all --num-callers=500 ./run_test_script 
