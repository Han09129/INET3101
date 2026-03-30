#include <stdio.h>
#include <stdlib.h>

void to_base_n(unsigned int number, int base) {
    char digits[] = "0123456789abcdef";
    char buffer[65];
    int index = 0;

    if (base < 2 || base > 16) {
        fprintf(stderr, "Error: base must be in the range 2-16.\n");
        return;
    }

    if (base == 8) {
        printf("0");
    } else if (base == 16) {
        printf("0x");
    }

    if (number == 0) {
        printf("0\n");
        return;
    }

    while (number > 0) {
        buffer[index++] = digits[number % base];
        number /= base;
    }

    while (index > 0) {
        putchar(buffer[--index]);
    }
    putchar('\n');
}

int main(int argc, char *argv[]) {
    unsigned int number;
    int base;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <non-negative integer> <base 2-16>\n", argv[0]);
        return 1;
    }

    number = (unsigned int)strtoul(argv[1], NULL, 10);
    base = atoi(argv[2]);

    printf("Base 10 input: %u\n", number);
    printf("Converted output: ");
    to_base_n(number, base);

    return (base < 2 || base > 16) ? 1 : 0;
}