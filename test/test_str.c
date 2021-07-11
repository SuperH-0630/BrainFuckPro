#include <stdlib.h>
#include "brainfuck.h"


int main() {
    int status;
    bf_env *env = bf_setEnv();
    bf_code code = bf_parserBrainFuck_Str("++++++++++[>+++++++>++++++++++>+++>+<<<<-]\n"
                                          ">++.>+.+++++++..+++.>++.<<+++++++++++++++.\n"
                                          ">.+++.------.--------.>+.>.");
    status = bf_runBrainFuck(code, env);
    bf_printError("error", env);
    bf_freeBrainFuck(code);

    bf_freeEnv(env);
    return status;
}