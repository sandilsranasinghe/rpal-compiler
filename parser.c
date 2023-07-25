#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Define ASTNodeType as an enum
typedef enum
{
    RESERVED,
    OPERATOR,
    IDENTIFIER,
    L_PAREN,
    // Add other token types as needed
} TokenType;

typedef enum
{
    ASTNodeType_LET,
    ASTNodeType_LAMBDA,
    ASTNodeType_WHERE,
    ASTNodeType_TAU,
    ASTNodeType_AUG

    // Add other node types as needed
} ASTNodeType;

typedef struct
{
    TokenType type;
    char *value;
} Token;

void procE()
{
    printf("procE\n");
    if (isCurrentToken(RESERVED, "let"))
    {
        readNT();
        procD();
        if (!isCurrentToken(RESERVED, "in"))
            printf("E: 'in' expected\n");
        readNT();
        procE();
        buildNAryASTNode(ASTNodeType_LET, 2);
    }
    else if (isCurrentToken(RESERVED, "fn"))
    {
        int treesToPop = 0;
        readNT();
        while (isCurrentToken(IDENTIFIER) || isCurrentToken(L_PAREN))
        {
            procVB();
            treesToPop++;
        }
        if (treesToPop == 0)
            printf("E: at least one 'Vb' expected\n");

        if (!isCurrentToken(OPERATOR, "."))
            printf("E: '.' expected\n");

        readNT();
        procE();
        buildNAryASTNode(ASTNodeType_LAMBDA, treesToPop + 1);
    }
    else
    {
        procEW();
    }
}

void procEW()
{
    procT();
    // extra readToken done in procT()
    if (isCurrentToken(RESERVED, "where"))
    {
        readNT();
        procDR();
        buildNAryASTNode(ASTNodeType_WHERE, 2);
    }
}

void procT()
{
    printf("procT\n");

    procTA();
    // extra readToken() in procTA()
    int treesToPop = 0;
    while (isCurrentToken(OPERATOR, ","))
    {
        readNT();
        procTA();
        // extra readToken() done in procTA()
        treesToPop++;
    }
    if (treesToPop > 0)
        buildNAryASTNode(ASTNodeType_TAU, treesToPop + 1);
}

void procTA()
{
    procTC();
    // extra readNT done in procTC()
    while (isCurrentToken(RESERVED, "aug"))
    {
        readNT();
        procTC();
        // extra readNT done in procTC()
        buildNAryASTNode(ASTNodeType_AUG, 2);
    }
}