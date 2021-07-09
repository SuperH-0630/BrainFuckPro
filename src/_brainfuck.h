#ifndef BRAINFUCK__BRAINFUCK_H
#define BRAINFUCK__BRAINFUCK_H

#define TABLE_SIZE (20)
#define FILE_READ_SIZE (20)

typedef char *bf_byte;
typedef struct bf_item bf_item;
typedef struct bf_pitem bf_pitem;
typedef struct bf_env bf_env;

#ifndef TYPEDEF_bf_env
typedef struct bf_env bf_env;
#define TYPEDEF_bf_env
#endif

#ifndef TYPEDEF_bf_code
typedef struct bf_code *bf_code;
#define TYPEDEF_bf_code
#endif

struct bf_code {
    bf_byte byte;
};

struct bf_item {
    int data[TABLE_SIZE];
    struct bf_item *prev;
    struct bf_item *next;
};

struct bf_pitem {
    int index;
    struct bf_item *item;
    struct bf_item *base;
};

struct bf_env {
    struct bf_item *item;
    struct bf_pitem pitem;
    char *error_info;  // 错误信息 (字符串常量)
};

#endif //BRAINFUCK__BRAINFUCK_H
