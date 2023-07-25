#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static struct lex_state LEX_STATE;
static char *ALL_PUNCTUATION = "(),;=&|~+-*/>";
static int token_count;

static void get_next_char(FILE *fp);
static void increment_line();

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
    LEX_STATE.line_number = 1;
    LEX_STATE.column_number = 0;

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

    get_next_char(fp);
    while (LEX_STATE.current_char != EOF)
    {
        // read identifier
        if (isalpha(LEX_STATE.current_char))
        {
            token = read_identifier(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // read integer
        else if (isdigit(LEX_STATE.current_char))
        {
            token = read_integer(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // read comment or punctuation
        // '/' might be an operator or the beginning of a comment
        else if (LEX_STATE.current_char == '/')
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
        else if (strchr(ALL_PUNCTUATION, LEX_STATE.current_char) != NULL)
        {
            token = read_punctuation(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // read string
        else if (LEX_STATE.current_char == '\'')
        {
            token = read_string(fp);
            tokens[token_count] = token;
            token_count++;
        }
        // discard whitespace and EOL characters
        else if (LEX_STATE.current_char == '\n')
        {
            increment_line();
            get_next_char(fp);
        }
        else if (isspace(LEX_STATE.current_char))
        {
            get_next_char(fp);
        }
        // unrecognized character
        else
        {
            printf("Error: unrecognized character %c at line %d col %d\n",
                   LEX_STATE.current_char,
                   LEX_STATE.line_number,
                   LEX_STATE.column_number-1);
            // TODO we might want to finish scanning the file before exiting to catch all errors
            exit(0);
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
    int i = 0;

    while (LEX_STATE.current_char != EOF)
    {
        if (isalpha(LEX_STATE.current_char) || isdigit(LEX_STATE.current_char) || LEX_STATE.current_char == '_')
        {
            identifier[i] = LEX_STATE.current_char;
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

        get_next_char(fp);
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
    int i = 0;

    while (LEX_STATE.current_char != EOF)
    {
        if (isdigit(LEX_STATE.current_char))
        {
            integer[i] = LEX_STATE.current_char;
            i++;
        }
        else if (isalpha(LEX_STATE.current_char))
        {
            printf("Error: invalid integer at line %d col %d\n",
                   LEX_STATE.line_number,
                   LEX_STATE.column_number-1);
            exit(0);
        }
        else
        {
            break;
        }

        if (i >= int_size)
        {
            // TODO how to handle this? Also check if there is an actual limit
            printf("Error: integer too large at line %d col %d\n",
                   LEX_STATE.line_number,
                   LEX_STATE.column_number-1);
            exit(0);
        }

        get_next_char(fp);
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

// NOTE that this should only be called when LEX_STATE.current_char is a '/'
int is_comment(FILE *fp)
{
    // Assuming only single line comments are possible
    get_next_char(fp);
    int _is_comment = LEX_STATE.current_char == '/';
    if (!_is_comment)
    {
        fseek(fp, -1, SEEK_CUR);
        LEX_STATE.column_number -= 1;
    }
    return _is_comment;
}

void read_comment(FILE *fp)
{
    while (LEX_STATE.current_char != EOF)
    {
        if (LEX_STATE.current_char == '\n')
        {
            increment_line();
            break;
        }
        get_next_char(fp);
    }
}

// Note that we will be reading only a single punctuation per token here. `->` would be read
// as 2 tokens here and will need to be handled later during screening
struct rpal_token *read_punctuation(FILE *fp)
{
    char *punctuation = malloc(sizeof(char) * 2);
    punctuation[0] = LEX_STATE.current_char;
    punctuation[1] = '\0';

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_PUNCTUATION;
    token->tkn_value = punctuation;

    get_next_char(fp);

    return token;
}

struct rpal_token *read_string(FILE *fp)
{
    // read the rest of the string
    int str_size = 8;
    char *string = malloc(sizeof(char) * (str_size + 1));
    string[0] = LEX_STATE.current_char;
    int i = 1;
    get_next_char(fp);

    while (LEX_STATE.current_char != EOF)
    {
        if (LEX_STATE.current_char == '\'')
        {
            string[i] = LEX_STATE.current_char;
            i++;
            break;
        }
        else
        {
            string[i] = LEX_STATE.current_char;
            i++;
        }

        if (i > str_size - 2)
        {
            str_size += 8;
            string = realloc(string, sizeof(char) * (str_size + 1));
        }

        get_next_char(fp);
    }
    string[i] = '\0';

    struct rpal_token *token = malloc(sizeof(struct rpal_token));
    token->tkn_type = RPAL_TOKEN_STRING;
    token->tkn_value = string;

    get_next_char(fp);

    return token;
}

struct rpal_token **screen(struct rpal_token **tokens)
{
    struct rpal_token **screened_tokens = malloc(sizeof(struct rpal_token *) * (token_count + 1));
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

struct rpal_token *update_if_keyword(struct rpal_token *token)
{
    return token;
}

void get_next_char(FILE *fp)
{
    LEX_STATE.current_char = fgetc(fp);
    LEX_STATE.column_number++;
}

void increment_line()
{
    LEX_STATE.line_number++;
    LEX_STATE.column_number = 1;
}