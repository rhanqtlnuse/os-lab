#include <stdio.h>

void my_print(long color, char *format, ...);

int main(int argc, const char *argv[]) {
    my_print(0, "", "", "");
    printf("abc\n");
    my_print(1, "", 1, 1.5);
    printf("def\n");
    my_print(2, "g h i %d\n", 12);
    printf("ghi\n");

    return 0;
}