#ifndef BRAINFUCK_BRAINFUCK_H
#define BRAINFUCK_BRAINFUCK_H
#include <stdbool.h>

#ifndef TYPEDEF_bf_code
typedef struct bf_code *bf_code;
#define TYPEDEF_bf_code
#endif

#ifndef TYPEDEF_bf_env
typedef struct bf_env bf_env;
#define TYPEDEF_bf_env
#endif

#ifndef TYPEDEF_bf_STEP_FUNC
#define TYPEDEF_bf_STEP_FUNC
typedef int (*bf_STEP_FUNC)(bf_env *env);
#endif

#define bf_setEnvModeFunc(name) bool bf_setEnv##name##Mode(bf_env *env, int mode)
bf_setEnvModeFunc(step);
bf_setEnvModeFunc(information);
#undef bf_setEnvModeFunc

#define bf_setEnvMode(env, name, mode) (bf_setEnv##name##Mode((env), (mode)))
// bf_setEnvMode用于设置enc的mode
// 目前env的mode支持step(步进模式)和information(执行时显示信息)
// mode: 0-关闭 1-打开 -1-若打开则关闭，否则打开 其他值-无实际操作
// 返回值为对应mode的实际值

char *bf_getVersionInfo(void);  // 获取版本和版本信息字符串
char *bf_getVersion(void);  // 获取版本

// bf_env指针通常可以使用void *来代替
bf_env *bf_setEnv(void);  // 设置一个运行环境(返回一个指针)
void bf_freeEnv(bf_env *env);  // 释放一个环境
void bf_initEnv(bf_env *env);  // 复位读取头到初始位置

// bf_code指针通常可以使用void *来代替
void bf_freeBrainFuck(bf_code code);  // 释放代码
bf_code bf_parserBrainFuck_File(FILE *file);  // 读取文件并生成代码(返回一个code)
bf_code bf_parserBrainFuck_Str(const char *str); // 读取字符串并生成代码(返回一个code)
void bf_printBrainFuck(bf_code code); // 打印代码
int bf_runBrainFuck(bf_code code, bf_env *env); // 在指定环境中执行代码 (返回0表示无异常)
void bf_printError(char *info, bf_env *env);  // 打印错误信息, 若无错误则不执行
void bf_printPaperTape(bf_env *env);  // 打印纸带
void bf_printHead(bf_env *env);  // 打印读取头信息
void bf_printEnv(bf_env *env);  // 打印env信息
void bf_printEnvWithMode(bf_env *env);  // 打印env信息和env的mode信息
bf_STEP_FUNC bf_setEnvStepFunc(bf_env *env, bf_STEP_FUNC step_func);  // 设置步进函数, 每次不仅时当用户输入m时则回调该函数(step_func)

#endif //BRAINFUCK_BRAINFUCK_H
