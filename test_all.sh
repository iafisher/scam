#!/bin/bash

function invoke_valgrind {
	valgrind -q --leak-check=full --show-leak-kinds=all --num-callers=500 $1
}

function invoke_run_test_script {
	invoke_valgrind ./run_test_script $1
}

invoke_valgrind ./tests
invoke_run_test_script resources/test_stdlib.scm
invoke_run_test_script resources/test_core.scm
invoke_run_test_script resources/test_escapes.scm
invoke_run_test_script resources/test_programs.scm
