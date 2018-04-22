#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * C中不能用const常量声明数组长度
 */
#define PARAM_COUNT 4

typedef struct _pl {
    char *params[PARAM_COUNT];
} param_list;

/**
 * @param list 只使用第一个参数，作为char *command
 */
void my_help(const param_list *list);

/**
 * 不考虑出现以“-p”和“-t”为路径名的情况，shell中将这样的字符串当作参数而不是路径名
 * 比如“ls -ahl”，即使当前目录下有名为“-ahl”的目录，也将“-ahl”作为参数来使用，
 * 这个函数与其一致
 * 
 * @param list 只使用前两个参数，分别作为char *format, char *path
 */
void my_ls(const param_list *list);

/**
 * @param list 只使用第一个参数，作为char *filename
 */
void my_cat(const param_list *list);

/**
 * @param list 只使用第一个参数，作为char *path
 */
void my_count(const param_list *list);

/**
 * @param list 不使用任何参数
 */
void my_exit(const param_list *list);

/**
 * 命令数
 */
const int COMMAND_COUNT = 5;

/**
 * 命令名
 */
const char *COMMAND_NAME[] = {
    "help",
    "ls",
    "cat",
    "count",
    "exit"
};

/**
 * 命令格式
 */
const char *COMMAND_FORMAT[] = {
    "help[ command]",
    "ls[ -p|-t][ path]",
    "cat <filename>",
    "count <path>",
    "exit"
};

/**
 * 命令说明（描述）
 */
const char *COMMAND_DESCRIPTION[] = {
    "缺省命令名时输出所有的命令及其说明，否则输出指定命令的说明",
    "p代表plain（普通输出），t代表tree（树形输出），缺省时使用plain格式；缺省路径名时列出当前目录下的普通文件和目录",
    "显示<filename>的内容",
    "递归地显示指定目录及其所有子目录下的普通文件数和目录数",
    "退出程序"
};

/**
 * 输入暂存区大小
 */
const int BUF_SIZE = 4095;

/**
 * 处理输入，是功能调用的入口函数
 * 
 * @param buffer 输入的命令和参数
 */
void process_input(char *buffer);

/**
 * 读取命令
 * 
 * @param buffer 暂存命令
 * @return 是否输入了命令
 */
bool read_command(char *buffer);

int main(int argc, const char *argv[]) {
    printf("OS_Lab_2: FAT12 Image Viewer\n\n");
    printf("如果您第一次使用这个程序，输入\"help\"以查看帮助；如果您忘记了某条命令的用法，输入\"help <command>\"以查看其说明。\n\n");
    
    char buffer[BUF_SIZE];
    while (true) {
        printf("> ");
        if (!read_command(buffer)) {
            process_input(buffer);
        }
    }

    return 0;
}

void my_help(const param_list *list) {
    if (list->params[0] != NULL) {
        int i;
        for (i = 0; i < COMMAND_COUNT && strcmp(list->params[0], COMMAND_NAME[i]) != 0; i++)
            ;
        if (i < COMMAND_COUNT) {
            printf("%s: \n", COMMAND_NAME[i]);
            printf("  格式: %s\n", COMMAND_FORMAT[i]);
            printf("  说明: \n    %s\n", COMMAND_DESCRIPTION[i]);
        } else {
            printf("Error: %s: command not found\n", list->params[0]);
        }
    } else {
        for (int i = 0; i < COMMAND_COUNT; i++) {
            printf("%s: \n", COMMAND_NAME[i]);
            printf("  格式: %s\n", COMMAND_FORMAT[i]);
            printf("  说明: \n    %s\n", COMMAND_DESCRIPTION[i]);
        }
    }
}
void my_ls(const param_list *list) {
    printf("ls\n");
}
void my_cat(const param_list *list) {
    printf("cat\n");
}
void my_count(const param_list *list) {
    printf("count\n");
}
void my_exit(const param_list *list) {
    exit(0);
}

void process_input(char *buffer) {
    /**
     * 功能函数入口
     */
    void (*COMMAND_ENTRY[])(const param_list *) = {
        my_help,
        my_ls,
        my_cat,
        my_count,
        my_exit
    };

    char *cmd = strtok(buffer, " ");
    int i;
    for (i = 0; i < COMMAND_COUNT && strcmp(cmd, COMMAND_NAME[i]) != 0; i++)
        ;
    if (i < COMMAND_COUNT) {
        param_list *list = (param_list *) malloc(sizeof(param_list));
        for (int j = 0; j < PARAM_COUNT; j++) {
            list->params[j] = NULL;
        }

        char *param;
        int counter = 0;
        while (param = strtok(NULL, " ")) {
            list->params[counter++] = param;
        }
        /********* for debug **********/
        printf("count: %d\n", counter);
        for (int k = 0; k < counter; k++) {
            printf("[%s]\n", list->params[k]);
        }
        /********* for debug **********/
        COMMAND_ENTRY[i](list);
    } else {
        printf("Error: %s: command not found\n", cmd);
    }
}

bool read_command(char *buffer) {
    bool empty_cmd = true;
    bool previous_space = true;
    int i = 0;
    char ch;
    /**
     * 使用回车结束输入
     */
    while ((ch = getchar()) != '\n') {
        /**
         * 下面所说的空白符包括空格和制表符，因为换行符已经被排除
         * 
         * 如果还未输入命令，即empty_cmd == true
         *   如果遇到非空白符，将其存入buffer并将empty_cmd和previous_space设为false  
         *   否则，忽略
         * 否则，
         *   如果上一个字符是空白字符
         *     如果当前字符是非空白字符，将当前字符存入buffer并将previous_space设为false
         *     否则，忽略
         *   否则，
         *     如果当前字符是空白字符，则在buffer中存入一个空格，并将previous_space设为true
         *     否则，将其存入buffer
         */
        if (empty_cmd) {
            if (!isspace(ch)) {
                buffer[i++] = ch;
                empty_cmd = false;
                previous_space = false;
            }
        } else {
            if (previous_space) {
                if (!isspace(ch)) {
                    buffer[i++] = ch;
                    previous_space = false;
                }
            } else {
                if (isspace(ch)) {
                    buffer[i++] = ' ';
                    previous_space = true;
                } else {
                    buffer[i++] = ch;
                }
            }
        }
    }
    buffer[i] = '\0';

    return empty_cmd;
}