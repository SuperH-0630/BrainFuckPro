#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "brainfuck.h"
#define COMMAND_LINE_STR_SIZE (20)

static struct option long_options[] = {
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"eval", required_argument, 0, 'e'},
        {0, 0, 0, 0}
};

void printUsage(char *name) {
    printf("Usage: %s[options] file..\n", name);
    printf("Options: \n");
    printf(" -e --eval\t\tRun code in a string\n");
    printf(" -v --version\t\tshow version\n");
    printf(" -h --help\t\tshow help\n\n");

    printf("\nFor more information, please see: \n");
    printf("github.com/SuperH-0630/BranchFuckPro\n");
}

void printVersion() {
    printf("%s", bf_getVersionInfo());
}

int runFile(int argc, char **argv, bf_env *env) {
    while (optind < argc) {
        FILE *file = fopen(argv[optind], "r");

        if (file == NULL) {
            perror("read file error");
            return 1;
        }

        bf_code code;
        code = bf_parserBrainFuck_File(file);
        bf_runBrainFuck(code, env);
        bf_printError("run error", env);
        bf_initEnv(env);
        bf_freeBrainFuck(code);
        fclose(file);

        optind++;
    }
    return 0;
}

int runCommandLine_(bf_env *env) {
    int size = COMMAND_LINE_STR_SIZE;
    int status;
    char *str = calloc(size + 1, sizeof(char ));
    fgets(str, COMMAND_LINE_STR_SIZE + 1, stdin);
    while (!strchr(str, '\n') && !feof(stdin) && !ferror(stdin)) {
        char *new = calloc(size + COMMAND_LINE_STR_SIZE + 1, sizeof(char ));
        strcpy(new, str);
        fgets(new + size, COMMAND_LINE_STR_SIZE + 1, stdin);
        free(str);
        str = new;
        size += COMMAND_LINE_STR_SIZE;
    }

    bf_code code;
    code = bf_parserBrainFuck_Str(str);
    status = bf_runBrainFuck(code, env);
    bf_printError("command line error", env);
    bf_freeBrainFuck(code);
    free(str);
    return status;
}

int runCommandLine(bf_env *env) {
    int ch;
    unsigned int count = 0;

    printf("BranchFuck %s (" __DATE__ ", " __TIME__ ")\n", bf_getVersion());
    printf("Welcome to ues BranchFuck CommandLine (Type 'q' to quit)\n");

    while (1) {
        if (feof(stdin) || ferror(stdin))
            return 1;
        printf("[%d] >", count);
        ch = getc(stdin);
        if (ch == 'q')
            return 0;
        else
            ungetc(ch, stdin);
        runCommandLine_(env);
        count++;
    }
}

int main(int argc, char **argv){
    int option_index = 0;
    int status;
    bf_env *env = bf_setEnv();

    while (1) {
        option_index = 0;
        int c = getopt_long (argc, argv, "vhe:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            case 'v':
                printVersion();
                return 0;
            case 'e': {
                bf_code code;
                code = bf_parserBrainFuck_Str((char *) optarg);
                bf_runBrainFuck(code, env);
                bf_printError("eval error", env);
                bf_freeBrainFuck(code);
                break;
            }
            default:
            case '?':
                printUsage(argv[0]);
                return 1;
        }
    }

    status = runFile(argc, argv, env);
    if (status != 0)
        return status;

    status = runCommandLine(env);
    if (status == 1) {
        printf("stdin error\n");
        return status;
    }

    printf("BranchFuckPro: bye~\n");
    return 0;
}
