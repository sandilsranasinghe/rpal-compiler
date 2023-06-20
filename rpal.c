// #include "tokenizer.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    // get file name from argv
    char *filename = argv[1];

    // open file
    FILE *fp = fopen(filename, "r");

    // read file
    char c;
    while ((c = fgetc(fp)) != EOF) {
        printf("%c", c);
    }

    // close file
    fclose(fp);

    return 0;
}