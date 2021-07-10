#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "brainfuck.h"
#include "_brainfuck.h"

#ifndef BF_VERSION
#define BF_VERSION "Magic Version"
#define BF_VERSION_INFO "This is a magical version because no one knows how magical he is."
#endif

static bf_code make_bf_code(void);
static void free_bf_code(bf_code code);
static int runBrainFuck(bf_byte *byte, bf_env *env);
static int runBrainFuck_(bf_byte *byte, bf_env *env);
static void freeItem(bf_item *item);
static bf_item *makeItem(void);
static void addItem(bf_item *global);

char *bf_getVersionInfo(void) {
    return BF_VERSION "\n" BF_VERSION_INFO "\n";
}

char *bf_getVersion(void) {
    return BF_VERSION;
}

static bf_code make_bf_code(void) {
    bf_code code = calloc(1, sizeof(struct bf_code));
    return code;
}

static void free_bf_code(bf_code code) {
    free(code->byte);
    free(code);
}

void bf_freeBrainFuck(bf_code code) {
    free_bf_code(code);
}

bf_code bf_parserBrainFuck_Str(const char *str) {
    char ch;
    int size = FILE_READ_SIZE;
    int index = 0;
    int str_index = 0;
    bf_code code = make_bf_code();
    code->byte = calloc(FILE_READ_SIZE + 1, sizeof(bf_byte));

    while ((ch = str[str_index++]) != '\0'){
        if (ch == '@') {  // 注释语句(忽略所有到行末, 或同行的@)
            while ((ch = str[str_index++]) != '\n' && ch != '@' && ch != '\0')
                continue;
            continue;
        } else if (ch != '>' && ch != '<' &&
                   ch != '.' && ch != ',' &&
                   ch != '+' && ch != '-' &&
                   ch != ':' && ch != ';' &&
                   ch != '[' && ch != ']' &&
                   ch != '?' &&!isalnum(ch)) {
            continue;  // 其他内容也被忽略
        }

        if (index > size) {
            bf_byte byte = calloc(size + FILE_READ_SIZE + 1, sizeof(bf_byte));
            strcpy(byte, code->byte);
            free(code->byte);
            code->byte = byte;
        }

        code->byte[index] = (char)ch;
        index++;
    }

    return code;
}

bf_code bf_parserBrainFuck_File(FILE *file) {
    int ch;
    int size = FILE_READ_SIZE;
    int index = 0;
    bf_code code = make_bf_code();
    code->byte = calloc(FILE_READ_SIZE + 1, sizeof(bf_byte));

    while ((ch = getc(file)) != EOF){
        if (ch == '@') {  // 注释语句(忽略所有到行末, 或同行的@)
            while ((ch = getc(file)) != '\n' && ch != '@' && ch != EOF)
                continue;
            continue;
        } else if (ch != '>' && ch != '<' &&
            ch != '.' && ch != ',' &&
            ch != '+' && ch != '-' &&
            ch != ':' && ch != ';' &&
            ch != '[' && ch != ']' && !isalnum(ch)) {
            continue;  // 其他内容也被忽略
        }

        if (index > size) {
            bf_byte byte = calloc(size + FILE_READ_SIZE + 1, sizeof(bf_byte));
            strcpy(byte, code->byte);
            free(code->byte);
            code->byte = byte;
        }

        code->byte[index] = (char)ch;
        index++;
    }

    return code;
}

void bf_printBrainFuck(bf_code code) {
    printf("%s", code->byte);
}

static bf_item *makeItem(void) {
    bf_item *item = calloc(1, sizeof(bf_item));
    return item;
}

static void freeItem(bf_item *item) {
    bf_item *bak;
    while (item != NULL) {
        bak = item->next;
        free(item);
        item = bak;
    }
}

bf_env *bf_setEnv(void) {
    bf_env *env = calloc(1, sizeof(bf_env));
    env->item = makeItem();
    env->pitem.item = env->item;
    env->pitem.index = 0;
    env->pitem.base = env->item;
    env->error_info = NULL;
    env->step_mode = false;
    env->information_mode = false;
    return env;
}

void bf_freeEnv(bf_env *env) {
    freeItem(env->item);
    free(env);
}

void bf_initEnv(bf_env *env) {
    env->pitem.item = env->pitem.base;
    env->pitem.index = 0;
    env->error_info = NULL;
}

static void addItem(bf_item *global) {
    assert(global != NULL);
    while (global->next != NULL)
        global = global->next;
    global->next = makeItem();
    global->next->prev = global;
}

int bf_runBrainFuck(bf_code code, bf_env *env) {
    bf_byte byte = code->byte;
    int status = runBrainFuck(&byte, env);

    if (status != 0)
        return 1;  // 返回1表示错误
    else {
        env->error_info = NULL;
        return 0;
    }
}

static int runBrainFuck(bf_byte *byte, bf_env *env) {
    while (**byte != '\0') {
        int status = runBrainFuck_(byte, env);
        if (status != 0)
            return status;
    }
    return 0;
}

static int runBrainFuck_(bf_byte *byte, bf_env *env) {
    bf_pitem *pitem = &(env->pitem);
    char *b = *byte;
    if (env->information_mode) {
        printf("Execute instructions: %c[%p]\n", *b, b);
        bf_printEnv(env);
    }

    switch (**byte) {
        case '>': {
            int num = 1;
            int times = 0;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = strtol((*byte + 1), &new, 10);
                *byte = new - 1;
                times = num / 20;
                num = num % 20;
            }

            while (times-- != 0) {
                if (pitem->item->next == NULL)
                    addItem(pitem->item);
                pitem->item = pitem->item->next;
            }

            pitem->index += num;
            if (pitem->index >= TABLE_SIZE) {
                pitem->index = pitem->index - TABLE_SIZE;
                if (pitem->item->next == NULL)
                    addItem(pitem->item);
                pitem->item = pitem->item->next;
            }
            break;
        }
        case '<': {
            int num = 1;
            int times = 0;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = strtol((*byte + 1), &new, 10);
                *byte = new - 1;
                times = num / 20;
                num = num % 20;
            }

            while (times-- != 0) {
                pitem->item = pitem->item->prev;
                if (pitem->item == NULL) {
                    pitem->item = makeItem();
                    pitem->item->next = env->item;
                    env->item->prev = pitem->item;
                    env->item = pitem->item;
                }
            }

            pitem->index -= num;
            if (pitem->index < -1) {
                pitem->index = (TABLE_SIZE + pitem->index);  // 如 pitem->index = -1, 即代表获取最后一个元素, 下标为 TABLE_SIZE - 1
                pitem->item = pitem->item->prev;
                if (pitem->item == NULL) {
                    pitem->item = makeItem();
                    pitem->item->next = env->item;
                    env->item->prev = pitem->item;
                    env->item = pitem->item;
                }
            }
            break;
        }
        case '+': {
            int num = 1;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = strtol((*byte + 1), &new, 10);
                *byte = new - 1;
            }
            pitem->item->data[pitem->index] += num;
            break;
        }
        case '-':{
            int num = 1;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = strtol((*byte + 1), &new, 10);
                *byte = new - 1;
            }
            pitem->item->data[pitem->index] -= num;
            break;
        }
        case ',':  // 输入
            pitem->item->data[pitem->index] = getc(stdin);
            if (pitem->item->data[pitem->index] != '\n' && pitem->item->data[pitem->index] != EOF) {
                int ch;
                while ((ch = getc(stdin)) != '\n' && ch != EOF)
                    continue;
            }
            break;
        case '.': {  // 输出
            int data = pitem->item->data[pitem->index];
            putc(data, stdout);
            fflush(stdout);
            break;
        }
        case ';': {  // 输入
            char num[6];
            char *end;
            fgets(num, 6, stdin);

            if (strchr(num, '\n') == NULL) {  // num 中不存在 \n
                int ch;
                while ((ch = getc(stdin)) != '\n' && ch != EOF)
                    continue;
            }

            pitem->item->data[pitem->index] = strtol(num, &end, 10);
            if (*end != '\0') {
                env->error_info = "':' instruction encountered an illegal number";
                return -1;
            }

            break;
        }
        case ':':  // 输出
            printf("%d", pitem->item->data[pitem->index]);
            break;
        case '?':
            bf_initEnv(env);
            break;
        case '[':
            while (1) {
                if (pitem->item->data[pitem->index] == 0) {  // 跳过代码直到遇到对应的]
                    int count = 1;
                    while (count > 0) {
                        (*byte)++;  // 读取下一个指令
                        if (**byte == '[')
                            count++;
                        else if (**byte == ']')
                            count--;
                        else if (**byte == '\0') {
                            env->error_info = "The ‘]’ was not encountered";
                            return -1;
                        }
                    }
                    break;  // 跳出最顶层循环 while (1)
                } else {  // 执行代码
                    int status;
                    bf_byte new = *byte + 1;  // 备份 获取下一个指令集
                    status = runBrainFuck(&new, env);  // 运行, 直到遇到]
                    if (status != 1) { // 既未遇到 ]
                        env->error_info = "The ']' was not encountered";
                        return -1;
                    }
                    env->error_info = NULL;  // 清除错误信息 "Illegal ']' encountered"
                }
            }
            break;
        case ']':
            (*byte)++;
            env->error_info = "Illegal ']' encountered";
            return 1;
        default:
            env->error_info = "unsupported command";
            return -1;
    }

    if (env->information_mode) {
        printf("\nEnd execute instructions: %c[%p]\n", *b, b);
        bf_printHead(env);
        printf("\n\n");
    }

    if (env->step_mode && env->step_func != NULL) {  // 步进模式
        int ch;
        while (1) {
            printf("(Type n or [enter] to continue. Type m to show menu.)\nEnter:");
            ch = getc(stdin);
            if (ch == 'n' || ch == '\n' || ch == EOF) { // 继续
                printf("continue...\n");
                break;
            } else if (ch == 'm') {  // 进入菜单
                if (env->step_func(env) != 0) {  // 调用函数 (stdin在该函数中清空)
                    env->error_info = "step mode func error";
                    return -1;
                }
            }
        }

        while ((ch = getc(stdin)) != '\n' && ch != EOF)  // 清除stdin
            continue;
    }

    (*byte)++;  // 读取下一个指令
    return 0;
}

void bf_printError(char *info, bf_env *env) {
    if (env->error_info != NULL)
        printf("%s : %s\n", info, env->error_info);
}

void bf_printPaperTape(bf_env *env) {
    unsigned count = 0;
    bf_item *item = env->item;
    while (true) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            printf("[(%d)%d", count, item->data[i]);
            if (isprint(item->data[i]))
                printf("'%c'", item->data[i]);
            if (i == env->pitem.index && item == env->pitem.item)
                printf("(head)");
            if (i == 0 && item == env->pitem.base)
                printf("(base)");
            printf("]");
            if (i != TABLE_SIZE - 1)  // 不是最后一个
                printf("-");
            count++;
        }
        item = item->next;
        if (item == NULL)
            break;
        else
            printf("-");  // 换行接着输出
    }
}

void bf_printHead(bf_env *env) {
    printf("head data: %d", env->pitem.item->data[env->pitem.index]);
    if (isprint(env->pitem.item->data[env->pitem.index]))
        printf("'%c'", env->pitem.item->data[env->pitem.index]);
}

void bf_printEnv(bf_env *env) {
    bf_printPaperTape(env);
    printf("\n");
    bf_printHead(env);
    printf("\n");
}

void bf_printEnvWithMode(bf_env *env) {
    bf_printPaperTape(env);
    printf("\n");
    bf_printHead(env);
    printf("\n");

    if (env->step_mode)  // 不修改实际值
        printf("step mode on\n");
    else
        printf("step mode off\n");

    if (env->information_mode)  // 不修改实际值
        printf("information mode on\n");
    else
        printf("information mode off\n");
}

bf_STEP_FUNC bf_setEnvStepFunc(bf_env *env, bf_STEP_FUNC step_func) {
    bf_STEP_FUNC bak = env->step_func;
    env->step_func = step_func;
    return bak;
}

#define bf_setEnvModeFunc(name) bool bf_setEnv##name##Mode(bf_env *env, int mode) { \
switch (mode) { \
case 1:env->name##_mode = true;break; \
case 0:env->name##_mode = false;break; \
case -1:env->name##_mode = !env->name##_mode;break; \
default:break; \
} \
return env->name##_mode; \
}

bf_setEnvModeFunc(step)
bf_setEnvModeFunc(information)
