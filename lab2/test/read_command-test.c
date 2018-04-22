#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

const int BUF_SIZE = 255;

bool read_command(char *buffer);

int main(int argc, const char *argv[]) {
    char buffer[BUF_SIZE];
    int i = 0;
    while (true) {
        printf(" In[%d]: ", i);
        bool empty_cmd = read_command(buffer);
        if (empty_cmd) {
            printf("Out[%d]: [empty]\n", i);
        } else {
            printf("Out[%d]: %s[end]\n", i, buffer);
        }
        i++;
    }

    return 0;
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
        if (ch == 'q') {
            exit(0);
        }
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