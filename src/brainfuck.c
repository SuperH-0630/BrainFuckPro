#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "brainfuck.h"
#include "_brainfuck.h"
#include "mem.h"

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
        if (ch == '#') {  // 注释语句(忽略所有到行末)
            while ((ch = str[str_index++]) != '\n' && ch != '\0')
                continue;
            continue;
        } else if (ch != '>' && ch != '<' &&  // 位移
                   ch != '.' && ch != ',' &&  // IO操作
                   ch != '+' && ch != '-' &&  // 数据操作
                   ch != ':' && ch != ';' &&  // IO操作
                   ch != '[' && ch != ']' &&  // 条件循环
                   ch != '?' && ch != '@' &&  // 重置、断点
                   !isalnum(ch)) {
            continue;  // 其他内容也被忽略
        }

        if (index >= size) {
            bf_byte byte = calloc(size + FILE_READ_SIZE + 1, sizeof(bf_byte));
            strcpy(byte, code->byte);
            free(code->byte);
            code->byte = byte;
            size += FILE_READ_SIZE;
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
        if (ch == '#') {  // 注释语句(忽略所有到行末)
            while ((ch = getc(file)) != '\n' && ch != '\0')
                continue;
            continue;
        } else if (ch != '>' && ch != '<' &&  // 位移
                   ch != '.' && ch != ',' &&  // IO操作
                   ch != '+' && ch != '-' &&  // 数据操作
                   ch != ':' && ch != ';' &&  // IO操作
                   ch != '[' && ch != ']' &&  // 条件循环
                   ch != '?' && ch != '@' &&  // 重置、断点
                   !isalnum(ch)) {
            continue;  // 其他内容也被忽略
        }

        if (index > size) {
            bf_byte byte = calloc(size + FILE_READ_SIZE + 1, sizeof(bf_byte));
            strcpy(byte, code->byte);
            free(code->byte);
            code->byte = byte;
            size += FILE_READ_SIZE;
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
    env->debug_mode = bf_no_debug;
    return env;
}


void bf_freeEnv(bf_env *env) {
    freeItem(env->item);
    free(env);
}


void bf_resetEnv(bf_env *env) {
    freeItem(env->item);

    env->item = makeItem();
    env->pitem.item = env->item;
    env->pitem.index = 0;
    env->pitem.base = env->item;
    env->error_info = NULL;
    env->step_mode = false;
    env->information_mode = false;
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

    env->debug_mode = bf_no_debug;
    if (status != 0)
        return status;  // 返回1表示错误
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
                num = (int)strtol((*byte + 1), &new, 10);
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

            assert(pitem->index >= 0);
            break;
        }
        case '<': {
            int num = 1;
            int times = 0;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = (int)strtol((*byte + 1), &new, 10);
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
            if (pitem->index < 0) {
                pitem->index = (TABLE_SIZE + pitem->index);  // 如 pitem->index = -1, 即代表获取最后一个元素, 下标为 TABLE_SIZE - 1
                pitem->item = pitem->item->prev;
                if (pitem->item == NULL) {
                    pitem->item = makeItem();
                    pitem->item->next = env->item;
                    env->item->prev = pitem->item;
                    env->item = pitem->item;
                }
            }

            assert(pitem->index >= 0);
            break;
        }
        case '+': {
            bf_data num = 1;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = (bf_data)strtol((*byte + 1), &new, 10);
                *byte = new - 1;
            }
            pitem->item->data[pitem->index] += num;
            break;
        }
        case '-':{
            bf_data num = 1;
            if (isalnum(*(*byte + 1))) {
                bf_byte new;
                num = (bf_data)strtol((*byte + 1), &new, 10);
                *byte = new - 1;
            }
            pitem->item->data[pitem->index] -= num;
            break;
        }
        case ',':  // 输入
            pitem->item->data[pitem->index] = (bf_data)getc(stdin);
            if (pitem->item->data[pitem->index] != '\n' && pitem->item->data[pitem->index] != EOF) {
                int ch;
                while ((ch = getc(stdin)) != '\n' && ch != EOF)
                    continue;
            }
            break;
        case '.': {  // 输出
            bf_data data = pitem->item->data[pitem->index];
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

            pitem->item->data[pitem->index] = (bf_data)strtol(num, &end, 10);
            if (*end != '\0') {
                env->error_info = "';' instruction encountered an illegal number";
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
        case '@':
            if (env->debug_mode == bf_no_debug)
                env->debug_mode = bf_in_debug;
            goto RETURN;
        default:
            env->error_info = "unsupported command";
            return -1;
    }

    if (env->information_mode) {
        printf("\nEnd execute instructions: %c[%p]\n", *b, b);
        bf_printHead(env);
        printf("\n\n");
    }

    if ((env->step_mode || env->debug_mode == bf_in_debug) && env->step_func != NULL) {  // 步进模式/调试模式
        int ch;

        bf_printHead(env);
        printf("\n");
        while (1) {
            printf("(Type n or [enter] to continue. Type m to show menu.)\n");
            if (env->debug_mode)
                printf("(Type j to jump to the next point. Type g to ignore all point.)\n");
            printf("Enter:");
            switch ((ch = getc(stdin))) {
                case 'n':
                case '\n':
                case EOF:
                    printf("continue...\n");
                    goto BREAK_WHILE;  // 跳出while(1)循环
                case 'm':
                    if (env->step_func(env) != 0)  // 调用函数 (stdin在该函数中清空)
                        return -2;  // 直接退出执行
                    break;
                case 'j':
                    if (env->debug_mode == bf_in_debug) {
                        env->debug_mode = bf_no_debug;
                        printf("jump...\n");
                        goto BREAK_WHILE;  // 跳出while(1)循环
                    }
                    break;
                case 'g':
                    if (env->debug_mode == bf_in_debug) {
                        env->debug_mode = bf_not_debug;
                        printf("continue...\n");
                        goto BREAK_WHILE;  // 跳出while(1)循环
                    }
                    break;
                default:
                    break;
            }
        }
        BREAK_WHILE:  // 跳出上面的循环

        while ((ch = getc(stdin)) != '\n' && ch != EOF)  // 清除stdin
            continue;
    }

RETURN:
    (*byte)++;  // 读取下一个指令
    return 0;
}


char *bf_printError(char *info, bf_env *env) {
    if (env->error_info != NULL)
        fprintf(stderr, "%s : %s\n", info, env->error_info);
    return env->error_info;
}


static void printTape(bf_env *env, bf_item *item, int i, long long count){
    printf("[(%lld)%d", count, item->data[i]);
    if (isprint(item->data[i]))
        printf("'%c'", item->data[i]);
    if (i == env->pitem.index && item == env->pitem.item)
        printf("(head)");
    if (i == 0 && item == env->pitem.base)
        printf("(base)");
    printf("]");
}


void bf_printPaperTape(bf_env *env) {
    long long count = 0;
    bf_item *item = env->item;
    while (true) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            printTape(env, item, i, count);
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


void bf_printPaperTapeNear(bf_env *env) {
    int first_index;
    int count;
    bf_item *item;

    count = -20;
    item = env->pitem.item;
    first_index = env->pitem.index - 20;
    if (first_index < 0) {
        if (item->prev != NULL) {
            first_index = TABLE_SIZE + first_index;
            item = item->prev;
        } else {
            first_index = 0;
            printf("env->pitem.index = %d\n", env->pitem.index);
            count = -(env->pitem.index);  // 保证第env->pitem.index时count为0
            printf("count = %d\n", count);
        }
    }

    for (int flat = 0; count <= 20; flat++) {
        int i = 0;
        if (flat == 0)
            i = first_index;

        for (; i < TABLE_SIZE && count <= 20; i++) {
            printTape(env, item, i, count);
            if (i != TABLE_SIZE - 1 && count != 20)  // 不是最后一个
                printf("-");
            count++;
        }
        item = item->next;
        if (item == NULL)
            break;
        else if (count != 20)  // 不是最后一个
            printf("-");  // 换行接着输出
    }

}


void bf_printHead(bf_env *env) {
    printf("head data: %d", env->pitem.item->data[env->pitem.index]);
    if (isprint(env->pitem.item->data[env->pitem.index]))
        printf("'%c'", env->pitem.item->data[env->pitem.index]);
}


void bf_printEnv(bf_env *env) {
    bf_printPaperTapeNear(env);
    printf("\n");
    bf_printHead(env);
    printf("\n");
}


void bf_printEnvWithMode(bf_env *env) {
    bf_printPaperTape(env);
    printf("\n");
    bf_printHead(env);
    printf("\n");

    if (env->step_mode)
        printf("step mode on\n");
    else
        printf("step mode off\n");

    if (env->information_mode)
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
