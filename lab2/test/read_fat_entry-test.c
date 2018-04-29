#include <stdio.h>

int read_fat_entry(FILE *fp, int next);

int main() {
    FILE *fp = fopen("a.img", "rb");
    if (fp == NULL) {
        printf("wrong\n");
        exit(-1);
    }
    fseek(fp, 0x200 + 3, 0);
    printf("%#x\n", read_fat_entry(fp, 3));

    return 0;
}

int read_fat_entry(FILE *fp, int next) {
    int offset;
    if (next % 2 == 1) {
        offset = (next - 1) * 3 / 2;
    } else {
        offset = next * 3 / 2;
    }
    printf("%d\n", offset);
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
