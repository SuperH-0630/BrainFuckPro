# BrainFuck Pro
使用C语言实现的BrainFuck解释器，并且扩展了部分语法。
## 关于BrainFuck
Brainfuck，是一种极小化的程序语言，它是由Urban Müller在1993年创造的。这种语言有时被简称为BF。
## 命令函参数
```
使用: BrainFuck[参数] 文件..  
参数:  
-e --eval              从一个字符串中运行代码  
-v --version           显示版本信息  
-h --help              显示帮助信息  
```
##支持的语法
```
使用：指令[指令[指令...]]
指令
>       读取头向后移动1格
<       读取头向前移动1格
+       读取头所指格子的数据增加1
-       读取头所指格子的写入数据减1

>n      读取头向后移动n格
<n      读取头向前移动n格
+n      读取头所指格子的数据增加n
-n      读取头所指格子的写入数据减n

.       输出读取头所指格子的数据(字符形式)
,       对读取头所指格子的写入数据(字符形式)
:       输出读取头所指格子的数据(数字形式)
;       对读取头所指格子的写入数据(数字形式)

[       循环初始位置（若读取头所指格子的数据不为0则循环）
]       循环结束位置
?       读取头回到初始位置
```

##命令行模式
执行完成指定文件后，BrainFuckPro进入命令行交互模式。使用者可以通过stdin即时写入指令操控读取头。
##关于API
BrainFuckPro为C语言提供了相应的API。若需要使用，请链接库`BrainFuck_LIB`和包含头文件`brainfuck.h`。  
```c
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
```
`bf_env`即图灵机的纸带，`bf_code`即读取头的指令。二者本质上均为指针，且在brainfuck.h中无具体实现，使用者可以使用`void *`来代替。
##构建
使用cmake+make来构建本项目。
cmake中可以设置`CMAKE_INSTALL_PREFIX`指定文件安装路径。
可以使用`make`构建项目，使用`make install`安装项目。
##声明
###版权声明
版权所有 © 2021 [SuperHuan](https://github.com/SuperH-0630) 保留所有权利。  
BrainFuckPro编程语言C解释器由SuperHuan开发，技术归属SuperHuan。  
本授权协议适用于BrainFuckPro编程语言C解释器，SuperHuan拥有对本授权协议最终解释权和修改权。  
###免责声明
BrainFuckPro编程语言C解释器为免费开源程序。  
编译、运行该程序过程中造成的损失（包括但不限于系统损坏、文件丢失）均由您个人承担， 与开发者无关。
###LICENSE
该代码是在 MIT License 下发布的。
