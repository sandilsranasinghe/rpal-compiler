#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Define ASTNodeType as an enum
typedef enum {
    IDENTIFIER,
    INTEGER,
    STRING,
    LET,
    LAMBDA,
    WHERE,
    TAU,
    AUG,
    CONDITIONAL,
    OR,
    AND,
    NOT,
    GR,
    GE,
    LS,
    LE,
    EQ,
    NE,
    PLUS,
    MINUS,
    MULT,
    DIV,
    EXP,
    AT,
    GAMMA,
    TRUE,
    FALSE,
    NIL,
    DUMMY,
    WITHIN,
    SIMULTDEF,
    REC,
    EQUAL,
    FCNFORM,
    PAREN,
} ASTNodeType;