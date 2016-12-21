/* ./test_eq <code> <expected result>
 *     Evaluate the code and expected result, return 0 if they are equal and
 *     1 otherwise
 */
#include <stdio.h>
#include "../builtins.h"
#include "../eval.h"
#include "../scamval.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./%s <code> <expected result>\n", argv[0]);
        return 2;
    } else {
        scamenv* env = scamenv_init(NULL);
        register_builtins(env);
        scamval* v1 = eval_str(argv[1], env);
        scamval* v2 = eval_str(argv[2], env);
        int result = !scamval_eq(v1, v2);
        scamval_free(v1);
        scamval_free(v2);
        scamenv_free(env);
        return result;
    }
}
