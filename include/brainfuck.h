#ifndef BRAINFUCK_BRAINFUCK_H
#define BRAINFUCK_BRAINFUCK_H

#ifndef TYPEDEF_bf_code
typedef struct bf_code *bf_code;
#define TYPEDEF_bf_code
#endif

#ifndef TYPEDEF_bf_env
typedef struct bf_env bf_env;
#define TYPEDEF_bf_env
#endif

char *bf_getVersionInfo();  // 获取版本和版本信息字符串
char *bf_getVersion();  // 获取版本

// bf_env指针通常可以使用void *来代替
bf_env *bf_setEnv();  // 设置一个运行环境(返回一个指针)
void bf_freeEnv(bf_env *env);  // 释放一个环境
void bf_initEnv(bf_env *env);  // 复位读取头到初始位置

// bf_code指针通常可以使用void *来代替
void bf_freeBrainFuck(bf_code code);  // 释放代码
bf_code bf_parserBrainFuck_File(FILE *file);  // 读取文件并生成代码(返回一个code)
bf_code bf_parserBrainFuck_Str(const char *str); // 读取字符串并生成代码(返回一个code)
void bf_printBrainFuck(bf_code code); // 打印代码
int bf_runBrainFuck(bf_code code, bf_env *env); // 在指定环境中执行代码 (返回0表示无异常)
void bf_printError(char *info, bf_env *env);  // 打印错误信息, 若无错误则不执行

#endif //BRAINFUCK_BRAINFUCK_H
