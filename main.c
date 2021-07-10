#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#include "brainfuck.h"
#define COMMAND_LINE_STR_SIZE (20)
#define FILE_NAME_SIZE (20)

static bf_env *global_env;
static struct option long_options[] = {
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"eval", required_argument, 0, 'e'},
        {"print", no_argument, 0, 'p'},
        {"step", no_argument, 0, 's'},
        {"information", no_argument, 0, 'i'},
        {"noninit", no_argument, 0, 'n'},
        {0, 0, 0, 0}
};
char *program_name;
bool print_code = false;
bool initEnv = true;  // 每次执行命令行参数指定的文件时都重置读取头位置

void printUsage(void);
void printVersion(void);
void printMenu(void);
int stepModeFunc(bf_env *env);
int runFile(int argc, char **argv, bf_env *env);
int runCommandLine_(bf_env *env);
int runCommandLine(bf_env *env);
void free_env_at_exit(void);
int clInformation(int ch, bf_env *env);

int main(int argc, char **argv){
    int option_index = 0;
    int status;
    program_name = *argv;
    global_env = bf_setEnv();
    atexit(free_env_at_exit);
    bf_setEnvStepFunc(global_env, stepModeFunc);

    while (1) {
        option_index = 0;
        int c = getopt_long (argc, argv, "vhsie:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                break;
            case 'h':
                printUsage();
                goto end;
            case 'v':
                printVersion();
                goto end;
            case 'e': {
                bf_code code;
                code = bf_parserBrainFuck_Str((char *) optarg);  // 不会回到头部重新执行

                if (print_code) {
                    printf("code: ");
                    bf_printBrainFuck(code);
                    printf("\n");
                }

                bf_runBrainFuck(code, global_env);
                bf_printError("eval error", global_env);
                bf_freeBrainFuck(code);
                printf("\n");
                break;
            }
            case 's':
                bf_setEnvMode(global_env, step, 1);
                break;
            case 'i':
                bf_setEnvMode(global_env, information, 1);
                break;
            case 'p':
                print_code = true;
                break;
            case 'n':
                initEnv = false;
                break;
            default:
            case '?':
                printUsage();
                return 1;
        }
    }

    status = runFile(argc, argv, global_env);
    if (status != 0)
        return status;

    status = runCommandLine(global_env);
    if (status == 1) {
        printf("stdin error\n");
        return status;
    }

    printf("BrainFuckPro: bye~\n");
    return 0;

    end:  // 从-v和-h中退出
    if (argc > 2) {
        printf("Too many argument\n");
        return 1;
    }
    return 0;
}


void printUsage(void) {
    printf("Usage: %s[options] file..\n", program_name);
    printf("Options: \n");
    printf(" -e --eval\t\t\tRun code in a string\n");
    printf(" -v --version\t\t\tShow version\n");
    printf(" -h --help\t\t\tShow help\n");
    printf(" -s --step\t\t\tRun with step mode\n");
    printf(" -i --information\t\tRun with information mode\n");
    printf(" -n --noninit\t\t\tDon't init env after run file\n\n");
    printf(" -p --print\t\t\tPrint code after parser\n\n");

    printf("CommandLine Options: \n");
    printf(" v show version\n");
    printf(" h show help\n");
    printf(" q quit\n");
    printf(" m show CommandLine Menu\n");
    printf("CommandLine Menu: \n");
    printMenu();

    printf("\n");
    printf("Step Options: \n");
    printf(" n continue\n");
    printf(" m show step menu\n");
    printf("Step Menu: \n");
    printMenu();

    printf("\n");
    printf(" For more information, please see: \n");
    printf(" github.com/SuperH-0630/BrainFuckPro\n");
}


void printVersion(void) {
    printf("%s", bf_getVersionInfo());
}


void printMenu(void) {
    printf("+---+----------------------------+\n");
    printf("+ v + show version               +\n");
    printf("+ h + show help                  +\n");
    printf("+ m + show menu                  +\n");
    printf("+ d + print code after parser    +\n");
    printf("+ w + print env information      +\n");
    printf("+ p + print paper tape           +\n");
    printf("+ r + print read head            +\n");
    printf("+ s + run in step model          +\n");
    printf("+ i + run with information       +\n");
    printf("+ n + set new env                +\n");
    printf("+ f + run file                   +\n");
    printf("+ c + clear screen(May not work) +\n");
    printf("+ q + quit                       +\n");
    printf("+ e + exit the menu              +\n");
    printf("+---+----------------------------+\n");
}


int runFile(int argc, char **argv, bf_env *env) {
    while (optind < argc) {
        FILE *file = fopen(argv[optind], "r");

        if (file == NULL) {
            perror("read file error");
            return 1;
        }

        bf_code code;
        if (initEnv)
            bf_initEnv(env);  // 每次执行文件都回到头部重新执行
        code = bf_parserBrainFuck_File(file);

        if (print_code) {
            printf("code: ");
            bf_printBrainFuck(code);
            printf("\n");
        }

        bf_runBrainFuck(code, env);
        bf_printError("run error", env);
        bf_freeBrainFuck(code);
        fclose(file);
        printf("\n");

        optind++;
    }
    return 0;
}


int stepModeFunc(bf_env *env) {
    return clInformation('m', env);
}


int clInformation(int ch, bf_env *env) {
    int return_ = 0;
    switch (ch) {
        case 'v':
            printVersion();
            break;
        case 'h':
            printUsage();
            break;
        case 'm': {
            printMenu();
            printf("Enter the operation:");

            int del_ch;
            while ((del_ch = getc(stdin) != '\n') && del_ch != EOF)
                continue;

            ch = getc(stdin);
            return_ = clInformation(ch, env);
            goto NOT_CLEAR;  // 不用清除stdin
        }
        case 'q':  // 退出菜单
            return_ = 1;
            break;
        case 'd':
            print_code = !print_code;
            if (print_code)
                printf("print code mode on\n");
            else
                printf("print code mode off\n");
            break;
        case 'w':
            bf_printEnvWithMode(env);
            break;
        case 'p':
            bf_printPaperTape(env);
            printf("\n");
            break;
        case 'r':
            bf_printHead(env);
            printf("\n");
            break;
        case 's':
            if (bf_setEnvMode(env, step, -1))
                printf("step mode on\n");
            else
                printf("step mode off\n");
            break;
        case 'i':
            if (bf_setEnvMode(env, information, -1))
                printf("information mode on\n");
            else
                printf("information mode off\n");
            break;
        case 'n':
            bf_resetEnv(global_env);
            break;
        case 'f': {
            int size = FILE_NAME_SIZE;
            char *str = calloc(size + 1, sizeof(char ));

            int del_ch;
            while ((del_ch = getc(stdin) != '\n') && del_ch != EOF)
                continue;

            printf("Enter file address:");
            fgets(str, FILE_NAME_SIZE + 1, stdin);
            while (!strchr(str, '\n') && !feof(stdin) && !ferror(stdin)) {
                char *new = calloc(size + FILE_NAME_SIZE + 1, sizeof(char ));
                strcpy(new, str);
                fgets(new + size, FILE_NAME_SIZE + 1, stdin);
                free(str);
                str = new;
                size += FILE_NAME_SIZE;
            }

            *strchr(str, '\n') = '\0';  // 去掉回车
            FILE *file = fopen(str, "r");

            if (file == NULL) {
                printf("file = '%s'\n", str);
                perror("read file error222");
                return -1;
            }

            bf_code code;
            code = bf_parserBrainFuck_File(file);

            if (print_code) {
                printf("code: ");
                bf_printBrainFuck(code);
                printf("\n");
            }

            return_ = bf_runBrainFuck(code, env);
            bf_printError("run error", env);
            bf_freeBrainFuck(code);
            fclose(file);
            free(str);
            printf("\n");
            goto NOT_CLEAR;
        }
        case 'c':
            system("clear");  // 清空
            break;
        case 'e':  // 退出菜单
            break;
        default:
            printf("Unsupported menu option\n");
            break;
    }

    int del_ch;
    while ((del_ch = getc(stdin) != '\n') && del_ch != EOF)
        continue;

NOT_CLEAR:
    return return_;
}


int runCommandLine(bf_env *env) {
    int ch;

    printf("BrainFuck %s (" __DATE__ ", " __TIME__ ")\n", bf_getVersion());
    printf("Welcome to ues BrainFuck CommandLine (Type 'q' to quit)\n");
    printf("Type v(version), h(help) and m(menu) for more information\n");

    for (unsigned count = 0; true;count++) {
        if (feof(stdin) || ferror(stdin))
            return 1;
        printf("[%d] >", count);
        ch = getc(stdin);
        if (ch == 'q')
            return 0;
        else if (ch == 'v' || ch == 'h' || ch == 'm') {
            if(clInformation(ch, env) == 1)
                return 0;  // quit
            continue;
        } else
            ungetc(ch, stdin);
        if (runCommandLine_(env) == -2)
            return 0;  // 退出
    }
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

    if (print_code) {
        printf("code: ");
        bf_printBrainFuck(code);
        printf("\n");
    }

    status = bf_runBrainFuck(code, env);
    bf_printError("command line error", env);
    bf_freeBrainFuck(code);
    free(str);
    printf("\n");
    return status;
}


void free_env_at_exit(void) {
    bf_freeEnv(global_env);
}
