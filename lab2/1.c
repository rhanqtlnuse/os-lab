#include <stdio.h>
#include <string.h>

int main(int argc, const char *argv[]) {
    char input[16];
    scanf("%s", input);
    printf("%s\n", strtok(input, "/"));
    printf("%s\n", input);
    for (int i = 0; i < 16; i++) {
        printf("%#2x ", input[i]);
    }

    return 0;
}