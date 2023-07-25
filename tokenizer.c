#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char CURRENT_CHAR;
static char *ALL_PUNCTUATION = "(),;=&|~+-*/>";
static int token_count;

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
    struct rpal_token **tokens = screen(scan(fp));
    return tokens;
}

// Do a preliminary scan of the code to recognize tokens as identifiers, integers, punctuation,
// or strings. NOTE that we will discard whitespace, comments and EOL characters here. This is the
// first step in the tokenization process.
struct rpal_token **scan(FILE *fp)
{
    token_count = 0;
    int array_size = 16;
    // TODO consider if we want to implement our own linked list structure instead
    struct rpal_token **tokens = malloc(sizeof(struct rpal_token *) * array_size);
    struct rpal_token *token;

    CURRENT_CHAR = fgetc(fp);
    while (CURRENT_CHAR != EOF)
    {
        // read identifier
        if (isalpha(CURRENT_CHAR))
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
        // discard whitespace and EOL characters
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
    int idt_size = 8;
    char *identifier = malloc(sizeof(char) * (idt_size + 1));
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

        if (i > idt_size - 2)
        {
            idt_size += 8;
            identifier = realloc(identifier, sizeof(char) * (idt_size + 1));
        }
    }

    identifier[i] = '\0';

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_IDENTIFIER;
    token->tkn_value = identifier;

    return token;
}

struct rpal_token *read_integer(FILE *fp)
{
    // read the rest of the integer
    int int_size = 8;
    char *integer = malloc(sizeof(char) * (int_size + 1));
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

        if (i >= int_size) {
            // TODO how to handle this? Also check if there is an actual limit
            printf("Error: integer too large\n");
            exit(1);
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

// Note that we will be reading only a single punctuation per token here. `->` would be read
// as 2 tokens here and will need to be handled later during screening
struct rpal_token *read_punctuation(FILE *fp)
{
    char *punctuation = malloc(sizeof(char) * 2);
    punctuation[0] = CURRENT_CHAR;
    punctuation[1] = '\0';

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_PUNCTUATION;
    token->tkn_value = punctuation;

    CURRENT_CHAR = fgetc(fp);

    return token;
}

struct rpal_token *read_string(FILE *fp)
{
    // read the rest of the string
    int str_size = 8;
    char *string = malloc(sizeof(char) * (str_size + 1));
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

        if (i > str_size - 2)
        {
            str_size += 8;
            string = realloc(string, sizeof(char) * (str_size + 1));
        }
    }
    string[i] = '\0';

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_STRING;
    token->tkn_value = string;

    CURRENT_CHAR = fgetc(fp);

    return token;
}

struct rpal_token **screen(struct rpal_token **tokens)
{
    struct rpal_token **screened_tokens = malloc(sizeof(struct rpal_token *) * (token_count+1));
    struct rpal_token *token;

    for (int i = 0; i < token_count; i++)
    {
        token = tokens[i];
        switch (token->tkn_type)
        {
        case RPAL_TOKEN_IDENTIFIER:
            /* code */
            break;
        
        default:
            break;
        }
    }

    struct rpal_token *end_token = malloc(sizeof(struct rpal_token));
    end_token->tkn_type = RPAL_TOKEN_END;
    end_token->tkn_value = NULL;
    screened_tokens[token_count] = end_token;

    return tokens;
}

struct rpal_token *update_if_keyword(struct rpal_token *token) {

    return token;
}