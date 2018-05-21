#include <stdio.h>

int main(int argc, const char *argv[]) {
    FILE *fp = fopen("a.img", "rb");
    fputc(0x00, fp);
    
    return 0;
}