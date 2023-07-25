#include "tokenizer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct lex_state LEX_STATE;
static char ALL_PUNCTUATION[] = "(),;=|+-*/>";
static int token_count;
static int token_merged = 0;

static void get_next_char(FILE *fp);
static void increment_line();

static struct rpal_token *read_identifier(FILE *fp);
static struct rpal_token *read_integer(FILE *fp);
static struct rpal_token *read_string(FILE *fp);
static struct rpal_token *read_punctuation(FILE *fp);
static void read_comment(FILE *fp);

static struct rpal_token *screen_identifier(struct rpal_token *token);
static struct rpal_token *screen_punctuation(struct rpal_token *token,
                                             struct rpal_token *next_token);

static int is_comment(FILE *fp);
static struct rpal_token *copy_token(const struct rpal_token *token);

// Tokenize the code in the given file. This is the entry point for the
// tokenizer. Returns an array of tokens.
struct rpal_token **tokenize(FILE *fp)
{
    LEX_STATE.line_number = 1;
    LEX_STATE.column_number = 0;

    struct rpal_token **tokens = scan(fp);
    struct rpal_token **screened_tokens = screen((const struct rpal_token **)tokens);

    // return tokens;
    clean_tokens(tokens);

    return screened_tokens;
}

// Do a preliminary scan of the code to recognize tokens as identifiers,
// integers, punctuation, or strings. NOTE that we will discard whitespace,
// comments and EOL characters here. This is the first step in the tokenization
// process.
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
                   LEX_STATE.current_char, LEX_STATE.line_number,
                   LEX_STATE.column_number - 1);
            // TODO we might want to finish scanning the file before exiting to catch
            // all errors
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
        if (isalpha(LEX_STATE.current_char) || isdigit(LEX_STATE.current_char) ||
            LEX_STATE.current_char == '_')
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
                   LEX_STATE.line_number, LEX_STATE.column_number - 1);
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
                   LEX_STATE.line_number, LEX_STATE.column_number - 1);
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

// Note that we will be reading only a single punctuation per token here. `->`
// would be read as 2 tokens here and will need to be handled later during
// screening
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

struct rpal_token **screen(const struct rpal_token **tokens)
{
    struct rpal_token **screened_tokens =
        malloc(sizeof(struct rpal_token *) * (token_count + 1));
    int skipped = 0;
    int i;
    token_merged = 0;

    for (i = 0; i + skipped < token_count; i++)
    {
        if (token_merged)
        {
            token_merged = 0;
            skipped += 1;
        }

        switch (tokens[i + skipped]->tkn_type)
        {
        case RPAL_TOKEN_IDENTIFIER:
            screened_tokens[i] = screen_identifier(copy_token((const struct rpal_token *)tokens[i + skipped]));
            break;

        case RPAL_TOKEN_PUNCTUATION:
            if (i + skipped + 1 < token_count) {
                screened_tokens[i] = screen_punctuation(
                copy_token((const struct rpal_token *)tokens[i + skipped]),
                copy_token((const struct rpal_token *)tokens[i + skipped + 1]));
            } else {
                screened_tokens[i] = copy_token((const struct rpal_token *)tokens[i + skipped]);
            }
            break;

        default:
            screened_tokens[i] = copy_token((const struct rpal_token *)tokens[i + skipped]);
            break;
        }
    }

    struct rpal_token *end_token = malloc(sizeof(struct rpal_token));
    end_token->tkn_type = RPAL_TOKEN_END;
    end_token->tkn_value = "$";
    screened_tokens[i] = end_token;

    screened_tokens = realloc(screened_tokens, sizeof(struct rpal_token *) * (i + 2));

    return screened_tokens;
}

// if token value matches a keyword, identify it as a keyword
struct rpal_token *screen_identifier(struct rpal_token *token)
{
    if (strcmp(token->tkn_value, "rec") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "where") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "in") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "and") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "let") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "fn") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "or") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "not") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "gr") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "ge") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "ls") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "le") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "eq") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "ne") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "within") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "true") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "false") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "nil") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "dummy") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    else if (strcmp(token->tkn_value, "aug") == 0)
    {
        token->tkn_type = RPAL_TOKEN_KEYWORD;
    }
    return token;
}

// identify punctuation tokens that are more than 1 character long
// identify operators and other punctuation
struct rpal_token *screen_punctuation(struct rpal_token *token,
                                      struct rpal_token *next_token)
{
    if (!strcmp(token->tkn_value, "-") &&
        (next_token->tkn_type == RPAL_TOKEN_PUNCTUATION &&
         !strcmp(next_token->tkn_value, ">")))
    {
        token->tkn_type = RPAL_TOKEN_OPERATOR;
        char *op = "->";
        token->tkn_value = op;
        token_merged = 1;
    }
    if (
        strcmp("(", token->tkn_value) &&
        strcmp(")", token->tkn_value) &&
        strcmp(",", token->tkn_value) &&
        strcmp(";", token->tkn_value))
    {
        token->tkn_type = RPAL_TOKEN_OPERATOR;
    }

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

struct rpal_token *copy_token(const struct rpal_token *token)
{
    struct rpal_token *new_token = malloc(sizeof(struct rpal_token));
    new_token->tkn_type = token->tkn_type;

    char *_tkn_value = malloc(sizeof(char) * (strlen(token->tkn_value) + 1));
    strcpy(_tkn_value, token->tkn_value);

    new_token->tkn_value = _tkn_value;
    new_token->line_number = token->line_number;
    new_token->column_number = token->column_number;

    return new_token;
}