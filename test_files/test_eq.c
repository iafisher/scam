/* ./test_eq <code> <expected result>
 *     Evaluate the code and expected result, return 0 if they are equal and
 *     1 otherwise
 */
#include <stdio.h>
#include "../eval.h"
#include "../scamval.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./%s <code> <expected result>\n", argv[0]);
        return 2;
    } else {
        scamval* env = scamdict_builtins();
        scamval* v1 = eval_str(argv[1], env);
        scamval* v2 = eval_str(argv[2], env);
        return !scamval_eq(v1, v2);
    }
}
