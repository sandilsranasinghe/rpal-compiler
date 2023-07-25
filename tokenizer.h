#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

enum rpal_token_type
{
    RPAL_TOKEN_IDENTIFIER,
    RPAL_TOKEN_INTEGER,
    RPAL_TOKEN_PUNCTUATION,
    RPAL_TOKEN_STRING,
    RPAL_TOKEN_KEYWORD,
    RPAL_TOKEN_OPERATOR
};

struct rpal_token
{
    enum rpal_token_type tkn_type;
    char *tkn_value;
};

struct rpal_token **tokenize(FILE *fp);
struct rpal_token **scan(FILE *fp);
struct rpal_token **screen(struct rpal_token **tokens);

void clean_tokens(struct rpal_token **tokens);

#endif // TOKENIZER_H