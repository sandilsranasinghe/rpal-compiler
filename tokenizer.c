#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char CURRENT_CHAR;
static char *ALL_PUNCTUATION = "(),;=&|~+-*/>";

static struct rpal_token *read_identifier(FILE *fp);
static struct rpal_token *read_integer(FILE *fp);
static struct rpal_token *read_string(FILE *fp);
static struct rpal_token *read_punctuation(FILE *fp);
static void read_comment(FILE *fp);

static int is_comment(FILE *fp);

// Tokenize the code in the given file. This is the entry point for the tokenizer.
// Returns an array of tokens.
struct rpal_token **tokenize(FILE *fp)
{
    struct rpal_token **tokens = scan(fp);

    return tokens;
}

// Do a preliminary scan of the code to recognize tokens as identifiers, integers, punctuation,
// or strings. NOTE that we will discard whitespace, comments and EOL characters here. This is the
// first step in the tokenization process.
struct rpal_token **scan(FILE *fp)
{
    int token_count = 0;
    int array_size = 16;
    // TODO consider if we want to implement our own linked list structure instead
    struct rpal_token **tokens = malloc(sizeof(struct rpal_token *) * array_size);
    struct rpal_token *token;

    CURRENT_CHAR = fgetc(fp);
    while (CURRENT_CHAR != EOF)
    {
        // read identifier
        if (isalpha(CURRENT_CHAR) | CURRENT_CHAR == '_')
        {
            token = read_identifier(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // read integer
        else if (isdigit(CURRENT_CHAR))
        {
            token = read_integer(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // read comment or punctuation
        // '/' might be an operator or the beginning of a comment
        else if (CURRENT_CHAR == '/')
        {
            // check if it's a comment
            // NOTE that we move the file pointer back if it's not a comment
            int _is_comment = is_comment(fp);
            if (_is_comment)
            {
                read_comment(fp);
            }
            else
            {
                token = read_punctuation(fp);
                tokens[token_count] = token;
                token_count++;
            }
        }
        // read punctuation
        else if (strchr(ALL_PUNCTUATION, CURRENT_CHAR) != NULL)
        {
            token = read_punctuation(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // read string
        else if (CURRENT_CHAR == '\'')
        {
            token = read_string(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // discard whitespace, comments and EOL characters
        else if (CURRENT_CHAR == '\n')
        {
            CURRENT_CHAR = fgetc(fp);
        }
        else if (isspace(CURRENT_CHAR))
        {
            CURRENT_CHAR = fgetc(fp);
        }
        // unrecognized character
        else
        {
            printf("Error: unrecognized character %c\n", CURRENT_CHAR);
            // TODO we might want to finish scanning the file before exiting to catch all errors
            exit(1);
        }

        if (token_count > array_size - 2)
        {
            array_size += 16;
            tokens = realloc(tokens, sizeof(struct rpal_token *) * array_size);
        }
    }

    return tokens;
}

struct rpal_token *read_identifier(FILE *fp)
{
    // read the rest of the identifier
    char *identifier = malloc(sizeof(char) * 8);
    identifier[0] = CURRENT_CHAR;
    int i = 1;

    while ((CURRENT_CHAR = fgetc(fp)) != EOF)
    {
        if (isalpha(CURRENT_CHAR) || isdigit(CURRENT_CHAR) || CURRENT_CHAR == '_')
        {
            identifier[i] = CURRENT_CHAR;
            i++;
        }
        else
        {
            break;
        }
    }

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_IDENTIFIER;
    token->tkn_value = identifier;

    return token;
}

struct rpal_token *read_integer(FILE *fp)
{
    // read the rest of the integer
    char *integer = malloc(sizeof(char) * 8);
    integer[0] = CURRENT_CHAR;
    int i = 1;

    while ((CURRENT_CHAR = fgetc(fp)) != EOF)
    {
        if (isdigit(CURRENT_CHAR))
        {
            integer[i] = CURRENT_CHAR;
            i++;
        }
        else
        {
            break;
        }
    }

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_INTEGER;
    token->tkn_value = integer;

    return token;
}

void clean_tokens(struct rpal_token **tokens)
{
    int i = 0;
    while (tokens[i] != NULL)
    {
        free(tokens[i]->tkn_value);
        free(tokens[i]);
        i++;
    }
    free(tokens);
}

// NOTE that this should only be called when CURRENT_CHAR is a '/'
int is_comment(FILE *fp)
{
    // Assuming only single line comments are possible
    CURRENT_CHAR = fgetc(fp);
    int _is_comment = CURRENT_CHAR == '/';
    if (!_is_comment)
    {
        fseek(fp, -1, SEEK_CUR);
    }
    return _is_comment;
}

void read_comment(FILE *fp)
{
    while ((CURRENT_CHAR = fgetc(fp)) != EOF)
    {
        if (CURRENT_CHAR == '\n')
        {
            break;
        }
    }
}

struct rpal_token *read_punctuation(FILE *fp)
{
    // read the rest of the punctuation
    char *punctuation = malloc(sizeof(char) * 3);
    punctuation[0] = CURRENT_CHAR;
    int i = 1;

    while ((CURRENT_CHAR = fgetc(fp)) != EOF)
    {
        if (strchr(ALL_PUNCTUATION, CURRENT_CHAR) != NULL)
        {
            punctuation[i] = CURRENT_CHAR;
            i++;
        }
        else
        {
            break;
        }
    }

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_PUNCTUATION;
    token->tkn_value = punctuation;

    return token;
}

struct rpal_token *read_string(FILE *fp)
{
    // read the rest of the string
    char *string = malloc(sizeof(char) * 8);
    string[0] = CURRENT_CHAR;
    int i = 1;

    while ((CURRENT_CHAR = fgetc(fp)) != EOF)
    {
        if (CURRENT_CHAR == '\'')
        {
            string[i] = CURRENT_CHAR;
            i++;
            break;
        }
        else
        {
            string[i] = CURRENT_CHAR;
            i++;
        }
    }

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_STRING;
    token->tkn_value = string;

    CURRENT_CHAR = fgetc(fp);

    return token;
}