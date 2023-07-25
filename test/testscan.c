#include <stdio.h>
#include "../tokenizer.h"

int main(int argc, char *argv[]) {
    // get file name from argv
    char *filename = argv[1];

    // open file
    FILE *fp = fopen(filename, "r");

    struct rpal_token **tokens = scan(fp);

    // print all token values
    int i = 0;
    while (tokens[i] != NULL) {
        printf("%s -- %d\n", tokens[i]->tkn_value, tokens[i]->tkn_type);
        i++;
    }

    // clear tokens from memory
    clean_tokens(tokens);

    // close file
    fclose(fp);

    return 0;
}