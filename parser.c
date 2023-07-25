#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define ASTNodeType as an enum
typedef enum
{
    RESERVED,
    OPERATOR,
    IDENTIFIER,
    L_PAREN,
    RESERVED,
    INTEGER,
    STRING,
    R_PAREN,
    // Add other token types as needed
} TokenType;

typedef enum
{
    ASTNodeType_LET,
    ASTNodeType_LAMBDA,
    ASTNodeType_WHERE,
    ASTNodeType_TAU,
    ASTNodeType_AUG,
    ASTNodeType_CONDITIONAL,
    ASTNodeType_OR,
    ASTNodeType_AND,
    ASTNodeType_NOT,
    ASTNodeType_GR,
    ASTNodeType_GE,
    ASTNodeType_LS,
    ASTNodeType_LE,
    ASTNodeType_EQ,
    ASTNodeType_NE,
    ASTNodeType_NEG,
    ASTNodeType_PLUS,
    ASTNodeType_MINUS,
    ASTNodeType_MULT,
    ASTNodeType_DIV,
    ASTNodeType_EXP,
    ASTNodeType_AT,
    ASTNodeType_GAMMA,
    ASTNodeType_TRUE,
    ASTNodeType_FALSE,
    ASTNodeType_NIL,
    ASTNodeType_DUMMY,
    ASTNodeType_WITHIN,
    ASTNodeType_SIMULTDEF,
    ASTNodeType_REC,
    // Add other node types as needed
} ASTNodeType;

typedef struct
{
    TokenType type;
    char *value;
} Token;

Token currentToken;

typedef int bool;
#define true 1
#define false 0

void procE()
{
    printf("procE\n");
    if (isCurrentToken(RESERVED, "let"))
    {
        readNT();
        procD();
        if (!isCurrentToken(RESERVED, "in"))
        {
            printf("E: 'in' expected\n");
            exit(0);
        }

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
        {
            printf("E: at least one 'Vb' expected\n");
            exit(0);
        }

        if (!isCurrentToken(OPERATOR, "."))
        {
            printf("E: '.' expected\n");
            exit(0);
        }

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

void procTC()
{
    printf("procTC\n");

    procB();
    // extra readNT in procBT()
    if (isCurrentToken(OPERATOR, "->"))
    {
        readNT();
        procTC();
        // extra readNT done in procTC
        if (!isCurrentToken(OPERATOR, "|"))
        {
            printf("TC: '|' expected\n");
            exit(0);
        }

        readNT();
        procTC();
        // extra readNT done in procTC
        buildNAryASTNode(ASTNodeType_CONDITIONAL, 3);
    }
}

void procB()
{
    printf("procB\n");

    procBT();
    // extra readNT in procBT()
    while (isCurrentToken(RESERVED, "or"))
    {
        readNT();
        procBT();
        buildNAryASTNode(ASTNodeType_OR, 2);
    }
}

void procBT()
{
    printf("procBT\n");

    procBS();
    // extra readNT in procBS()
    while (isCurrentToken(OPERATOR, "&"))
    {
        readNT();
        procBS();
        // extra readNT in procBS()
        buildNAryASTNode(ASTNodeType_AND, 2);
    }
}

void procBP()
{
    printf("procBP\n");

    procA();
    // Bp -> A('gr' | '>' ) A => 'gr'
    if (isCurrentToken(RESERVED, "gr") || isCurrentToken(OPERATOR, ">"))
    {
        readNT();
        procA();
        // extra readNT in procA()
        buildNAryASTNode(ASTNodeType_GR, 2);
    }
    // Bp -> A ('ge' | '>=') A => 'ge'
    else if (isCurrentToken(RESERVED, "ge") || isCurrentToken(OPERATOR, ">="))
    {
        readNT();
        procA();
        // extra readNT in procA()
        buildNAryASTNode(ASTNodeType_GE, 2);
    }
    // Bp -> A ('ls' | '<' ) A => 'ls'
    else if (isCurrentToken(RESERVED, "ls") || isCurrentToken(OPERATOR, "<"))
    {
        readNT();
        procA();
        // extra readNT in procA()
        buildNAryASTNode(ASTNodeType_LS, 2);
    }
    // Bp -> A ('le' | '<=') A => 'le'
    else if (isCurrentToken(RESERVED, "le") || isCurrentToken(OPERATOR, "<="))
    {
        readNT();
        procA();
        // extra readNT in procA()
        buildNAryASTNode(ASTNodeType_LE, 2);
    }
    // Bp -> A 'eq' A => 'eq'
    else if (isCurrentToken(RESERVED, "eq"))
    {
        readNT();
        procA();
        // extra readNT in procA()
        buildNAryASTNode(ASTNodeType_EQ, 2);
    }
    // Bp -> A 'ne' A => 'ne'
    else if (isCurrentToken(RESERVED, "ne"))
    {
        readNT();
        procA();
        // extra readNT in procA()
        buildNAryASTNode(ASTNodeType_NE, 2);
    }
}

void procBS()
{
    printf("procBS\n");

    // Bs -> 'not' Bp => 'not'
    if (isCurrentToken(RESERVED, "not"))
    {
        readNT();
        procBP();
        // extra readNT in procBP()
        buildNAryASTNode(ASTNodeType_NOT, 1);
    }
    else
    {
        procBP();
        // Bs -> Bp
        // extra readNT in procBP()
    }
}

void procAF()
{
    printf("procAF\n");

    procAP();
    // Af -> Ap;
    // extra readNT in procAP()
    if (isCurrentToken(OPERATOR, "**"))
    { // Af -> Ap '**' Af => '**'
        readNT();
        procAF();
        buildNAryASTNode(ASTNodeType_EXP, 2);
    }
}

void procAT()
{
    printf("procAT\n");

    procAF();
    // At -> Af;
    // extra readNT in procAF()
    bool mult = true;
    while (isCurrentToken(OPERATOR, "*") || isCurrentToken(OPERATOR, "/"))
    {
        if (strcmp(currentToken.value, "*") == 0)
            mult = true;
        else if (strcmp(currentToken.value, "/") == 0)
            mult = false;
        readNT();
        procAF();
        // extra readNT in procAF()
        if (mult) // At -> At '*' Af => '*'
            buildNAryASTNode(ASTNodeType_MULT, 2);
        else // At -> At '/' Af => '/'
            buildNAryASTNode(ASTNodeType_DIV, 2);
    }
}

void procA()
{
    printf("procA\n");

    if (isCurrentToken(OPERATOR, "+"))
    { // A -> '+' At
        readNT();
        procAT();
        // extra readNT in procAT()
    }
    else if (isCurrentToken(OPERATOR, "-"))
    { // A -> '-' At => 'neg'
        readNT();
        procAT();
        // extra readNT in procAT()
        buildNAryASTNode(ASTNodeType_NEG, 1);
    }
    else
    {
        procAT();
        // A -> At
        // extra readNT in procAT()
    }

    bool plus = true;
    while (isCurrentToken(OPERATOR, "+") || isCurrentToken(OPERATOR, "-"))
    {
        if (strcmp(currentToken.value, "+") == 0)
            plus = true;
        else if (strcmp(currentToken.value, "-") == 0)
            plus = false;
        readNT();
        procAT();
        // extra readNT in procAT()
        if (plus) // A -> A '+' At => '+'
            buildNAryASTNode(ASTNodeType_PLUS, 2);
        else // A -> A '-' At => '-'
            buildNAryASTNode(ASTNodeType_MINUS, 2);
    }
}

void procAP()
{
    printf("procAP\n");

    procR();
    // Ap -> R;
    // extra readNT in procR()
    while (isCurrentToken(OPERATOR, "@"))
    { // Ap -> Ap '@' '<IDENTIFIER>' R => '@'
        readNT();
        if (!isCurrentTokenType(IDENTIFIER))
            printf("AP: expected Identifier\n"); // Replace with appropriate error handling
        readNT();
        procR();
        // extra readNT in procR()
        buildNAryASTNode(ASTNodeType_AT, 3);
    }
}

void procRN()
{
    printf("procRN\n");

    if (isCurrentTokenType(IDENTIFIER) ||
        isCurrentTokenType(INTEGER) ||
        isCurrentTokenType(STRING))
    {
        // R -> '<IDENTIFIER>', R -> '<INTEGER>', R-> '<STRING>'
        // No need to do anything, as these are already processed in procR()
    }
    else if (isCurrentToken(RESERVED, "true"))
    { // R -> 'true' => 'true'
        createTerminalASTNode(ASTNodeType_TRUE, "true");
        readNT();
    }
    else if (isCurrentToken(RESERVED, "false"))
    { // R -> 'false' => 'false'
        createTerminalASTNode(ASTNodeType_FALSE, "false");
        readNT();
    }
    else if (isCurrentToken(RESERVED, "nil"))
    { // R -> 'nil' => 'nil'
        createTerminalASTNode(ASTNodeType_NIL, "nil");
        readNT();
    }
    else if (isCurrentTokenType(L_PAREN))
    {
        readNT();
        procE(); // extra readNT in procE()
        if (!isCurrentTokenType(R_PAREN))
        {
            printf("RN: ')' expected\n"); // Replace with appropriate error handling
        }
        readNT();
    }
    else if (isCurrentToken(RESERVED, "dummy"))
    { // R -> 'dummy' => 'dummy'
        createTerminalASTNode(ASTNodeType_DUMMY, "dummy");
        readNT();
    }
}

void procR()
{
    printf("procR\n");

    procRN(); // R -> Rn; NO extra readNT in procRN(). See while loop below for reason.
    readNT();
    while (isCurrentTokenType(INTEGER) ||
           isCurrentTokenType(STRING) ||
           isCurrentTokenType(IDENTIFIER) ||
           isCurrentToken(RESERVED, "true") ||
           isCurrentToken(RESERVED, "false") ||
           isCurrentToken(RESERVED, "nil") ||
           isCurrentToken(RESERVED, "dummy") ||
           isCurrentTokenType(L_PAREN))
    {             // R -> R Rn => 'gamma'
        procRN(); // NO extra readNT in procRN(). This is important because if we do an extra
                  // readNT in procRN and currentToken happens to
                  // be an INTEGER, IDENTIFIER, or STRING, it will get pushed on the stack. Then,
                  // the GAMMA node that we build will have the
                  // wrong kids. There are workarounds, e.g. keeping the extra readNT in procRN()
                  // and checking here if the last token read
                  // (which was read in procRN()) is an INTEGER, IDENTIFIER, or STRING and, if so,
                  // to pop it, call buildNAryASTNode, and then
                  // push it again. I chose this option because it seems cleaner.
        buildNAryASTNode(ASTNodeType_GAMMA, 2);
        readNT();
    }
}

void procDR()
{
    printf("procDR\n");

    if (isCurrentToken(RESERVED, "rec"))
    { // Dr -> 'rec' Db => 'rec'
        readNT();
        procDB(); // extra readToken() in procDB()
        buildNAryASTNode(ASTNodeType_REC, 1);
    }
    else
    {             // Dr -> Db
        procDB(); // extra readToken() in procDB()
    }
}

void procDA()
{
    printf("procDA\n");

    procDR(); // Da -> Dr
    // extra readToken() in procDR()
    int treesToPop = 0;
    while (isCurrentToken(RESERVED, "and"))
    { // Da -> Dr ('and' Dr)+ => 'and'
        readNT();
        procDR(); // extra readToken() in procDR()
        treesToPop++;
    }
    if (treesToPop > 0)
    {
        buildNAryASTNode(ASTNodeType_SIMULTDEF, treesToPop + 1);
    }
}

void procD()
{
    printf("procD\n");

    procDA(); // D -> Da
    // extra readToken() in procDA()
    if (isCurrentToken(RESERVED, "within"))
    { // D -> Da 'within' D => 'within'
        readNT();
        procD();
        buildNAryASTNode(ASTNodeType_WITHIN, 2);
    }
}