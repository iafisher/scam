#include <stdio.h>
#include <time.h>
#include "eval.h"
#include "parse.h"
#include "scamval.h"

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;
    ScamDict* env = scamdict_builtins();
    size_t reps = 10;
    double shortest = 666;
    for (size_t i = 0; i < reps; i++) {
        ScamExpr* ast = parse_file(argv[1]);
        clock_t begin = clock();
        eval((ScamVal*)ast, env);
        clock_t end = clock();
        double this = (end - begin + 0.0) / CLOCKS_PER_SEC;
        shortest = (this < shortest) ? this : shortest;
    }
    char buf[70];
    time_t t = time(NULL);
    strftime(buf, sizeof buf, "%b %d %I:%S %p", localtime(&t));
    printf("%s: %s in %f secs\n", buf, argv[1], shortest);
    return 0;
}
