/* ./test_err <code>
 *     Evaluate the code and return 0 if it causes an error, 1 otherwise
 */
#include "../eval.h"
#include "../scamval.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./%s <code>\n", argv[0]);
        return 2;
    } else {
        scamval* env = scamenv_default();
        scamval* v = eval_str(argv[1], env);
        int result = (v->type != SCAM_ERR);
        scamval_free(v);
        scamenv_free(env);
        return result;
    }
}
