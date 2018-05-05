/**
 * 日后扩展：
 *   1. 实现指定输出格式（比如-p普通输出格式、-t树形输出格式）
 *   2. 实现cd、mkdir、rm、touch等在Linux中常用的命令
 */
//  输出格式：
//  ┣━ house
//  ┃  ┣━ room
//  ┃  ┃  ┣━ kitchen
//  ┃  ┃  ┃  ┗━ path.txt
//  ┃  ┃  ┗━ bed.txt
//  ┃  ┣━ table.txt
//  ┃  ┗━ chair.txt
//  ┣━ animal
//  ┃  ┣━ bird
//  ┃  ┗━ cat.txt
//  ┣━ river.txt
//  ┗━ mountain.txt
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
 * @param list 只使用第一个参数，作为char *format, char *path
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
    "help [command]",
    "ls [path]",
    "cat <filename>",
    "count <path>",
    "exit"
};

/**
 * 命令说明（描述）
 */
const char *COMMAND_DESCRIPTION[] = {
    "缺省命令名时输出所有的命令及其说明，否则输出指定命令的说明",
    "缺省路径名时列出根目录下的普通文件和目录，否则列出指定目录下的普通文件和目录",
    "显示<filename>的内容",
    "递归地显示指定目录及其所有子目录下的普通文件数和目录数",
    "退出程序"
};

/**
 * 输入暂存区大小
 */
const int BUF_SIZE = 4095;

const int FAT1_BEGIN = 0x200;

/**
 * 根分区起始地址
 */
const int ROOT_DIR_BEGIN = 0x2600;

const int DATA_BEGIN = 0x4200;

enum color {
    DEFAULT_COLOR,
    ORDINARY_COLOR,
    DIRECTORY_COLOR,
    DEBUG_COLOR,
    ERROR_COLOR
};

void my_print(long color, char *format, ...);

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

void read_ordinary_name(FILE *fp, char *name);
void read_directory_name(FILE *fp, char *name);
int read_fat_entry(FILE *fp, int next);

/**
 * 队列
 */
#define QUEUE_SIZE 256

int address_queue[QUEUE_SIZE];
int address_head = 0;
int address_tail = -1;

char *path_queue[QUEUE_SIZE];
int path_head = 0;
int path_tail = -1;

void enqueue_address(int v);
int dequeue_address();
void enqueue_path(char *path);
char * dequeue_path();
bool empty();

void clear_on_finish();

int main(int argc, const char *argv[]) {
    my_print(DEFAULT_COLOR, "OS_Lab_2: FAT12 Image Viewer\n\n");

    char buffer[BUF_SIZE];
    while (true) {
        my_print(0, "> ");
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

/**
 * 层序遍历
 * 
 * 1. 遇到目录
 *   1.1 输出名字
 *   1.2 读取其对应的开始簇号
 *   1.3 根据簇号找到其FAT项
 *     1.3.1 如果FAT项大于等于0xFF8，则继续搜索当前目录
 *     1.3.2 如果FAT项指明某个簇、则从该簇开始重复整个过程（注意非根目录的前两项是“.”和“..”）
 * 2. 遇到普通文件
 *   2.1 输出名字
 *   2.2 继续下一项
 */
void my_ls(const param_list *list) {
    FILE *img = fopen("a.img", "rb");
    char entryname[14];
    if (list->params[0] == NULL) {
        my_print(DEFAULT_COLOR, "/: \n");
        int k;
        for (k = ROOT_DIR_BEGIN; k < DATA_BEGIN; k += 0x20) {
            fseek(img, k + 0x0B, 0);
            int attribute = fgetc(img);
            if (attribute == 0x10) {
                fseek(img, k, 0);
                read_directory_name(img, entryname);
                my_print(DIRECTORY_COLOR, entryname);
                my_print(DEFAULT_COLOR, " ");
                fseek(img, k + 0x1A, 0);
                int firstCluster = fgetc(img) + (fgetc(img) << 8);
                enqueue_address(firstCluster);
                char *tmp = (char *) malloc(11);
                tmp[0] = '/';
                strcat(tmp, entryname);
                strcat(tmp, "/");
                enqueue_path(tmp);
            } else if (attribute == 0x20) {
                fseek(img, k, 0);
                read_ordinary_name(img, entryname);
                my_print(ORDINARY_COLOR, entryname);
                my_print(DEFAULT_COLOR, " ");
            } else {
                break;
            }
        }
        my_print(DEFAULT_COLOR, "\n\n");
        bool continued = false;
        bool emptyCluster = true;
        while (!empty()) {
            if (address_queue[address_head] < 0xFF8) {
                emptyCluster = true;
                long offset;
                if (continued) {
                    offset = DATA_BEGIN + (address_queue[address_head] - 2) * 0x200;
                } else {
                    offset = DATA_BEGIN + (address_queue[address_head] - 2) * 0x200 + 2 * 0x20;
                    my_print(DEFAULT_COLOR, path_queue[path_head]);
                    my_print(DEFAULT_COLOR, ": \n");
                }
                int i;
                int upper;
                if (continued) {
                    upper = 0x200;
                } else {
                    upper = 0x1c0;
                }
                for (i = 0; i < upper; i += 0x20) {
                    fseek(img, offset + i + 0x0B, 0);
                    int attribute = fgetc(img);
                    if (attribute == 0x10) {
                        fseek(img, offset + i, 0);
                        read_directory_name(img, entryname);
                        my_print(DIRECTORY_COLOR, entryname);
                        emptyCluster = false;
                        my_print(DEFAULT_COLOR, " ");
                        fseek(img, offset + i + 0x1A, 0);
                        int firstCluster = fgetc(img) + (fgetc(img) << 8);
                        enqueue_address(firstCluster);
                        char *tmp = (char *) malloc(255);
                        strcat(tmp, path_queue[path_head]);
                        strcat(tmp, entryname);
                        strcat(tmp, "/");
                        enqueue_path(tmp);
                    } else if (attribute == 0x20) {
                        fseek(img, offset + i, 0);
                        read_ordinary_name(img, entryname);
                        my_print(ORDINARY_COLOR, entryname);
                        my_print(DEFAULT_COLOR, " ");
                        emptyCluster = false;
                    } else {
                        break;
                    }
                }
                if (i >= upper) {
                    continued = true;
                } else {
                    continued = false;
                }
                address_queue[address_head] = read_fat_entry(img, address_queue[address_head]);
            } else {
                my_print(DEFAULT_COLOR, "\n");
                if (!emptyCluster) {
                    my_print(DEFAULT_COLOR, "\n");
                }
                dequeue_address();
                dequeue_path();
                continued = false;
            }
        }
        clear_on_finish();
    } else {
        int cluster = 0;
        char *pathname = (char *) malloc(4096);
        strcpy(pathname, "/\0");
        char *tmp = strtok(list->params[0], "/");
        // 对根目录进行搜索
        int i;
        for (i = ROOT_DIR_BEGIN; i < DATA_BEGIN; i += 0x20) {
            fseek(img, i + 0x0B, 0);
            int attribute = fgetc(img);
            if (attribute == 0x10) {
                fseek(img, i, 0);
                read_directory_name(img, entryname);
                if (strcmp(entryname, tmp) == 0) {
                    fseek(img, i + 0x1A, 0);
                    cluster = fgetc(img) + (fgetc(img) << 8);
                    strcat(pathname, tmp);
                    strcat(pathname, "/");
                    break;
                }
            } else if (attribute == 0x20) {
                fseek(img, i, 0);
                read_ordinary_name(img, entryname);
                if (strcmp(entryname, tmp) == 0) {
                    my_print(DEFAULT_COLOR, tmp);
                    my_print(DEFAULT_COLOR, ": 不是一个目录\n");
                    clear_on_finish();
                    return;
                }
            } else {
                my_print(DEFAULT_COLOR, tmp);
                my_print(DEFAULT_COLOR, ": 路径不存在\n");
                my_print(DEBUG_COLOR, "[head:%d] ", path_head);
                my_print(DEBUG_COLOR, "[tail:%d]\n", path_tail);
                for (int j = 0; j <= path_tail; j++) {
                    my_print(DEBUG_COLOR, path_queue[j]);
                    my_print(DEBUG_COLOR, "\n");
                }
                clear_on_finish();
                return;
            }
        }
        if (i >= DATA_BEGIN) {
            my_print(DEFAULT_COLOR, tmp);
            my_print(DEFAULT_COLOR, ": 路径不存在\n");
            clear_on_finish();
            return;
        }
        int offset;
        bool continued = false;
        bool found = false;
        while (true) {
            if (!continued) {
                tmp = strtok(NULL, "/");
                if (tmp == NULL) {
                    break;
                }
            }
            offset = DATA_BEGIN + (cluster - 2) * 0x200;
            int i;
            for (i = 0; i < 0x200; i += 0x20) {
                fseek(img, offset + i + 0x0B, 0);
                int attribute = fgetc(img);
                if (attribute == 0x10) {
                    fseek(img, offset + i, 0);
                    read_directory_name(img, entryname);
                    if (strcmp(entryname, tmp) == 0) {
                        fseek(img, offset + i + 0x1A, 0);
                        cluster = fgetc(img) + (fgetc(img) << 8);
                        strcat(pathname, tmp);
                        strcat(pathname, "/");
                        break;
                    }
                } else if (attribute == 0x20) {
                    fseek(img, offset + i, 0);
                    read_ordinary_name(img, entryname);
                    if (strcmp(entryname, tmp) == 0) {
                        my_print(DEFAULT_COLOR, tmp);
                        my_print(DEFAULT_COLOR, ": 不是一个目录\n");
                        clear_on_finish();
                        return;
                    }
                } else {
                    my_print(DEFAULT_COLOR, tmp);
                    my_print(DEFAULT_COLOR, ": 路径不存在\n");
                    clear_on_finish();
                    return;
                }
            }
            if (i >= 0x200) {
                cluster = read_fat_entry(img, cluster);
                continued = true;
            } else {
                continued = false;
            }
        }
        enqueue_address(cluster);
        enqueue_path(pathname);
        // 开始进行遍历
        bool emptyCluster = false;
        continued = false;
        while (!empty()) {
            if (address_queue[address_head] < 0xFF8) {
                emptyCluster = true;
                long offset;
                if (continued) {
                    offset = DATA_BEGIN + (address_queue[address_head] - 2) * 0x200;
                } else {
                    offset = DATA_BEGIN + (address_queue[address_head] - 2) * 0x200 + 2 * 0x20;
                    my_print(DEFAULT_COLOR, path_queue[path_head]);
                    my_print(DEFAULT_COLOR, ": \n");
                }
                int i;
                int upper;
                if (continued) {
                    upper = 0x200;
                } else {
                    upper = 0x1c0;
                }
                for (i = 0; i < upper; i += 0x20) {
                    fseek(img, offset + i + 0x0B, 0);
                    int attribute = fgetc(img);
                    if (attribute == 0x10) {
                        fseek(img, offset + i, 0);
                        read_directory_name(img, entryname);
                        my_print(DIRECTORY_COLOR, entryname);
                        emptyCluster = false;
                        my_print(DEFAULT_COLOR, " ");
                        fseek(img, offset + i + 0x1A, 0);
                        int firstCluster = fgetc(img) + (fgetc(img) << 8);
                        enqueue_address(firstCluster);
                        char *tmp = (char *) malloc(255);
                        strcat(tmp, path_queue[path_head]);
                        strcat(tmp, entryname);
                        strcat(tmp, "/");
                        enqueue_path(tmp);
                    } else if (attribute == 0x20) {
                        fseek(img, offset + i, 0);
                        read_ordinary_name(img, entryname);
                        my_print(ORDINARY_COLOR, entryname);
                        my_print(DEFAULT_COLOR, " ");
                        emptyCluster = false;
                    } else {
                        break;
                    }
                }
                if (i >= upper) {
                    continued = true;
                } else {
                    continued = false;
                }
                address_queue[address_head] = read_fat_entry(img, address_queue[address_head]);
            } else {
                my_print(DEFAULT_COLOR, "\n");
                if (!emptyCluster) {
                    my_print(DEFAULT_COLOR, "\n");
                }
                dequeue_address();
                dequeue_path();
                continued = false;
            }
        }
    }
}
void my_cat(const param_list *list) {
    if (list->params[0] == NULL) {
        my_print(ERROR_COLOR, "Error: ");
        my_print(DEFAULT_COLOR, "请输入路径\n");
        return;
    }
    char *path = (char *) malloc(1024);
    strcpy(path, list->params[0]);
    FILE *img = fopen("a.img", "rb");
    char *tmp = strtok(path, "/");
    char entryname[14];
    int cluster = 0;
    int length = 0;
    bool fileEncountered = false;
    for (int i = ROOT_DIR_BEGIN; i < DATA_BEGIN; i += 0x20) {
        fseek(img, i + 0x0B, 0);
        int attribute = fgetc(img);
        if (attribute == 0x10) {
            fseek(img, i, 0);
            read_directory_name(img, entryname);
            if (strcmp(entryname, tmp) == 0) {
                fseek(img, i + 0x1A, 0);
                cluster = fgetc(img) + (fgetc(img) << 8);
                break;
            }
        } else if (attribute == 0x20) {
            fseek(img, i, 0);
            read_ordinary_name(img, entryname);
            if (strcmp(entryname, tmp) == 0) {
                fileEncountered = true;
                fseek(img, i + 0x1A, 0);
                cluster = fgetc(img) + (fgetc(img) << 8);
                length = fgetc(img) + (fgetc(img) << 8) + (fgetc(img) << 16) + (fgetc(img) << 24);
                break;
            }
        } else {
            my_print(ERROR_COLOR, "Error: ");
            my_print(DEFAULT_COLOR, tmp);
            my_print(DEFAULT_COLOR, ": 路径不存在\n");
            clear_on_finish();
            return;
        }
    }
    int offset;
    int upper;
    bool found = false;
    bool continued = false;
    while (true) {
        if (!continued) {
            tmp = strtok(NULL, "/");
            if (tmp == NULL) {
                break;
            } else {
                if (fileEncountered) {
                    my_print(ERROR_COLOR, "Error: ");
                    my_print(DEFAULT_COLOR, list->params[0]);
                    my_print(DEFAULT_COLOR, ": 不是一个目录\n");
                    clear_on_finish();
                    return;
                }
            }
        }
        if (continued) {
            offset = DATA_BEGIN + (cluster - 2) * 0x200;
            upper = 0x200;
        } else {
            offset = DATA_BEGIN + (cluster - 2) * 0x200 + 2 * 0x20;
            upper = 0x1c0;
        }
        int i;
        for (i = 0; i < upper; i += 0x20) {
            fseek(img, offset + i + 0x0B, 0);
            int attribute = fgetc(img);
            if (attribute == 0x10) {
                fseek(img, offset + i, 0);
                read_directory_name(img, entryname);
                if (strcmp(entryname, tmp) == 0) {
                    fseek(img, offset + i + 0x1A, 0);
                    cluster = fgetc(img) + (fgetc(img) << 8);
                    break;
                }
            } else if (attribute == 0x20) {
                fseek(img, offset + i, 0);
                read_ordinary_name(img, entryname);
                if (strcmp(entryname, tmp) == 0) {
                    fileEncountered = true;
                    fseek(img, offset + i + 0x1A, 0);
                    cluster = fgetc(img) + (fgetc(img) << 8);
                    length = fgetc(img) + (fgetc(img) << 8) + (fgetc(img) << 16) + (fgetc(img) << 24);
                    break;
                }
            } else {
                my_print(ERROR_COLOR, "Error: ");
                my_print(DEFAULT_COLOR, list->params[0]);
                my_print(DEFAULT_COLOR, ": 路径不存在\n");
                clear_on_finish();
                return;
            }
        }
        if (i >= upper) {
            continued = true;
            cluster = read_fat_entry(img, cluster);
        } else {
            continued = false;
        }
    }
    printf("[cluster:%#x] ", cluster);
    printf("[length:%#x]\n", length);
    // 读取
    char buffer[513] = {'\0'};
    offset = DATA_BEGIN + (cluster - 2) * 0x200;
    fseek(img, offset, 0);
    int i;
    for (i = 0; length > 0; i++, length--) {
        buffer[i % 513] = fgetc(img);
        if (i % 513 == 511) {
            buffer[512] = '\0';
            my_print(DEFAULT_COLOR, buffer);
            cluster = read_fat_entry(img, cluster);
            offset = DATA_BEGIN + (cluster - 2) * 0x200;
            fseek(img, offset, 0);
            i++;
        }
    }
    buffer[i % 513] = '\0';
    my_print(DEFAULT_COLOR, buffer);
}
void my_count(const param_list *list) {
    FILE *img = fopen("a.img", "rb");
    char entryname[14];
    if (list->params[0] == NULL) {
        my_print(ERROR_COLOR, "Error: ");
        my_print(DEFAULT_COLOR, "请输入路径\n");
        return;
    }
    int cluster = 0;
    char *pathname = (char *) malloc(4096);
    strcpy(pathname, "/\0");
    char *tmp = strtok(list->params[0], "/");
    // 对根目录进行搜索
    for (int i = ROOT_DIR_BEGIN; i < DATA_BEGIN; i += 0x20) {
        fseek(img, i + 0x0B, 0);
        int attribute = fgetc(img);
        if (attribute == 0x10) {
            fseek(img, i, 0);
            read_directory_name(img, entryname);
            if (strcmp(entryname, tmp) == 0) {
                fseek(img, i + 0x1A, 0);
                cluster = fgetc(img) + (fgetc(img) << 8);
                strcat(pathname, tmp);
                strcat(pathname, "/");
                break;
            }
        } else if (attribute == 0x20) {
            fseek(img, i, 0);
            read_ordinary_name(img, entryname);
            if (strcmp(entryname, tmp) == 0) {
                my_print(ERROR_COLOR, "Error: ");
                my_print(DEFAULT_COLOR, tmp);
                my_print(DEFAULT_COLOR, ": 不是一个目录\n");
                clear_on_finish();
                return;
            }
        } else {
            my_print(ERROR_COLOR, "Error: ");
            my_print(DEFAULT_COLOR, tmp);
            my_print(DEFAULT_COLOR, ": 路径不存在\n");
            clear_on_finish();
            return;
        }
    }
    int offset;
    bool continued = false;
    bool found = false;
    while (true) {
        if (!continued) {
            tmp = strtok(NULL, "/");
            if (tmp == NULL) {
                break;
            }
        }
        offset = DATA_BEGIN + (cluster - 2) * 0x200;
        int i;
        for (i = 0; i < 0x200; i += 0x20) {
            fseek(img, offset + i + 0x0B, 0);
            int attribute = fgetc(img);
            if (attribute == 0x10) {
                fseek(img, offset + i, 0);
                read_directory_name(img, entryname);
                if (strcmp(entryname, tmp) == 0) {
                    fseek(img, offset + i + 0x1A, 0);
                    cluster = fgetc(img) + (fgetc(img) << 8);
                    strcat(pathname, tmp);
                    strcat(pathname, "/");
                    break;
                }
            } else if (attribute == 0x20) {
                fseek(img, offset + i, 0);
                read_ordinary_name(img, entryname);
                if (strcmp(entryname, tmp) == 0) {
                    my_print(ERROR_COLOR, "Error: ");
                    my_print(DEFAULT_COLOR, tmp);
                    my_print(DEFAULT_COLOR, ": 不是一个目录\n");
                    clear_on_finish();
                    return;
                }
            } else {
                my_print(ERROR_COLOR, "Error: ");
                my_print(DEFAULT_COLOR, tmp);
                my_print(DEFAULT_COLOR, ": 路径不存在\n");
                clear_on_finish();
                return;
            }
        }
        if (i >= 0x200) {
            cluster = read_fat_entry(img, cluster);
            continued = true;
        } else {
            continued = false;
        }
    }
    enqueue_address(cluster);
    enqueue_path(pathname);
    // 开始遍历
    int level = 0;
    int fileCount = 0;
    int dirCount = 0;
    continued = false;
    while (!empty()) {
        if (address_queue[address_head] < 0xff8) {
            long offset;
            if (continued) {
                offset = DATA_BEGIN + (address_queue[address_head] - 2) * 0x200;
            } else {
                offset = DATA_BEGIN + (address_queue[address_head] - 2) * 0x200 + 2 * 0x20;
            }
            int upper;
            if (continued) {
                upper = 0x200;
            } else {
                upper = 0x1c0;
            }
            int i;
            for (i = 0; i < upper; i += 0x20) {
                fseek(img, offset + i + 0x0B, 0);
                int attribute = fgetc(img);
                if (attribute == 0x10) {
                    dirCount++;
                    fseek(img, offset + i, 0);
                    read_directory_name(img, entryname);
                    fseek(img, offset + i + 0x1A, 0);
                    int firstCluster = fgetc(img) + (fgetc(img) << 8);
                    enqueue_address(firstCluster);
                    char *tmp = (char *) malloc(255);
                    strcpy(tmp, "\0");
                    strcat(tmp, entryname);
                    enqueue_path(tmp);
                } else if (attribute == 0x20) {
                    fileCount++;
                } else {
                    break;
                }
            }
            if (i >= upper) {
                continued = true;
            } else {
                continued = false;
            }
            address_queue[address_head] = read_fat_entry(img, address_queue[address_head]);
        } else {
            for (int i = 0; i < level; i++) {
                my_print(DEFAULT_COLOR, "  ");
            }
            char *name = (char *) malloc(9);
            strcpy(name, path_queue[path_head]);
            int length = strlen(name);
            for (int i = length - 1; i >= 0; i--) {
                if (name[i] == '/') {
                    if (i == length - 1) {
                        name[i] = '\0';
                    } else {
                        name += (i + 1);
                        break;
                    }
                }
            }
            my_print(DEFAULT_COLOR, name);
            my_print(DEFAULT_COLOR, ": %d file(s)", fileCount);
            my_print(DEFAULT_COLOR, ", %d dir(s)\n", dirCount);
            dequeue_address();
            dequeue_path();
            level++;
            dirCount = 0;
            fileCount = 0;
            continued = false;
        }
    }
}
void my_exit(const param_list *list) {
    my_print(DEFAULT_COLOR, "Goodbye!\n");
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
        // printf("count: %d\n", counter);
        // for (int k = 0; k < counter; k++) {
        //     printf("[%s]\n", list->params[k]);
        // }
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

void read_ordinary_name(FILE *fp, char *name) {
    int i;
    int j;
    int byte;
    for (i = 0, j = 0; i < 8 && (byte = fgetc(fp)) != 0x20; i++, j++) {
        name[j] = (char) byte;
    }
    name[j++] = '.';
    // 略过空格，7的原因是第一个空格已经被读取过，现在指向第二个空格
    if (i < 8) {
        for (; i < 7; i++) {
            fgetc(fp);
        }
        for (; i < 10 && (byte = fgetc(fp)) != 0x20; i++, j++) {
            name[j] = (char) byte;
        }
    } else {
        for (; i < 11 && (byte = fgetc(fp)) != 0x20; i++, j++) {
            name[j] = (char) byte;
        }
    }
    name[j] = '\0';
}

void read_directory_name(FILE *fp, char *name) {
    int i;
    int byte;
    for (i = 0; i < 8 && (byte = fgetc(fp)) != 0x20; i++) {
        name[i] = (char) byte;
    }
    name[i] = '\0';
}
int read_fat_entry(FILE *fp, int next) {
    int offset;
    if (next % 2 == 1) {
        offset = (next - 1) * 3 / 2;
    } else {
        offset = next * 3 / 2;
    }
    fseek(fp, FAT1_BEGIN + offset, 0);
    int fatBits = fgetc(fp) + (fgetc(fp) << 8) + (fgetc(fp) << 16);
    if (next % 2 == 1) {
        int mask = 0xFFF000;
        fatBits = fatBits & mask;
        fatBits = fatBits >> 12;
    } else {
        int mask = 0x000FFF;
        fatBits = fatBits & mask;
    }
    return fatBits;
}

void enqueue_address(int v) {
    if (address_tail == QUEUE_SIZE - 1) {
        printf("error: address_queue is full\n");
        exit(-1);
    }
    address_queue[++address_tail] = v;
}
int dequeue_address() {
    if (address_tail < address_head) {
        printf("error: address_queue is empty\n");
        exit(-1);
    }
    return address_queue[address_head++];
}

void enqueue_path(char *path) {
    if (path_tail == QUEUE_SIZE - 1) {
        printf("error: path_queue is full\n");
        exit(-1);
    }
    path_queue[++path_tail] = path;
}
char * dequeue_path() {
    if (path_tail < path_head) {
        printf("error: path_queue is empty\n");
        exit(-1);
    }
    return path_queue[path_head++];
}
bool empty() {
    return address_head - address_tail == 1;
}

void clear_on_finish() {
    for (int i = 0; i <= path_tail; i++) {
        path_queue[i] = NULL;
    }
    address_head = 0;
    address_tail = -1;
    path_head = 0;
    path_tail = -1;
}