#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
    DELETE,
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
    ASTNodeType_COMMA,
    ASTNodeType_EQUAL,
    ASTNodeType_FCNFORM,
    ASTNodeType_PAREN
    // Add other node types as needed
} ASTNodeType;


// typedef int bool;
#define true 1
#define false 0

typedef struct {
    ASTNodeType type;
    char* value;
} ASTNode;

typedef struct {
    Token* array;
    int size;
    int capacity;
} Scanner;

typedef struct {
    Scanner* s;
    Token* currentToken;
    Stack* stack;
} Parser;

// Define token types (you may need to adjust these based on your language)
typedef enum {
    IDENTIFIER,
    INTEGER,
    STRING,
    DELETE // TokenType.DELETE from Java
} TokenType;

// Define AST node types (you may need to adjust these based on your language)
typedef enum {
    ASTNodeType_IDENTIFIER,
    ASTNodeType_INTEGER,
    ASTNodeType_STRING,
    // Add more node types as needed
} ASTNodeType;

// Token structure
typedef struct {
    TokenType type;
    char* value;
    int sourceLineNumber;
} Token;

// AST node structure
typedef struct ASTNode {
    ASTNodeType type;
    char* value;
    int sourceLineNumber;
    struct ASTNode* child;
    struct ASTNode* sibling;
} ASTNode;

// Stack structure
typedef struct {
    ASTNode** arr;
    int top;
    int capacity;
} Stack;

ASTNode* buildNAryASTNode(ASTNodeType type, int ariness);
bool isCurrentTokenType(TokenType type);
void readNT();
void procE();
void startParse();
ASTNode* buildAST();

// Global variables
static Stack stack;
static Token currentToken;

// Initialize the stack
void initStack(int capacity) {
    stack.arr = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
    stack.top = -1;
    stack.capacity = capacity;
}

// Push an element to the stack
void push(ASTNode* node) {
    stack.top++;
    stack.arr[stack.top] = node;
}

// Pop an element from the stack
ASTNode* pop() {
    ASTNode* node = stack.arr[stack.top];
    stack.top--;
    return node;
}

// Check if the stack is empty
bool isEmpty() {
    return stack.top == -1;
}

// Main function
int main() {
    // Initialize the stack with a suitable capacity (adjust if needed)
    initStack(100);

    // Initialize your Scanner and read tokens as needed
    // Assuming you have a function to read tokens called readNextToken()

    // Call buildAST() to start parsing and building the AST
    ASTNode* astRoot = buildAST();

    // Use the generated AST as needed (e.g., evaluate, generate code, etc.)

    // Free the memory used by the stack and AST
    free(stack.arr);
    // Free any other dynamically allocated memory

    return 0;
}

// Implementation of the functions
ASTNode* createTerminalASTNode(ASTNodeType type, char* value, int sourceLineNumber) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value;
    node->sourceLineNumber = sourceLineNumber;
    node->child = NULL;
    node->sibling = NULL;
    return node;
}

ASTNode* buildNAryASTNode(ASTNodeType type, int ariness) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = NULL;
    node->sourceLineNumber = -1;
    node->child = NULL;
    node->sibling = NULL;

    while (ariness > 0) {
        ASTNode* child = pop();
        if (node->child != NULL)
            child->sibling = node->child;
        node->child = child;
        node->sourceLineNumber = child->sourceLineNumber;
        ariness--;
    }

    return node;
}

bool isCurrentTokenType(TokenType type) {
    return (currentToken.type == type);
}

void readNT() {
    do {
        // currentToken = readNextToken();
    } while (isCurrentTokenType(DELETE));

    if (currentToken.value != NULL) {
        if (currentToken.type == IDENTIFIER) {
            ASTNode* node = createTerminalASTNode(ASTNodeType_IDENTIFIER, currentToken.value, currentToken.sourceLineNumber);
            push(node);
        } else if (currentToken.type == INTEGER) {
            ASTNode* node = createTerminalASTNode(ASTNodeType_INTEGER, currentToken.value, currentToken.sourceLineNumber);
            push(node);
        } else if (currentToken.type == STRING) {
            ASTNode* node = createTerminalASTNode(ASTNodeType_STRING, currentToken.value, currentToken.sourceLineNumber);
            push(node);
        }
    }
}

void startParse() {
    readNT();
    procE();
    if (currentToken.value != NULL)
        printf("Expected EOF.\n");
}

ASTNode* buildAST() {
    startParse();
    return pop();
}

ASTNode* createTerminalASTNode(ASTNodeType type, char* value, int sourceLineNumber) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value;
    node->sourceLineNumber = sourceLineNumber;
    node->child = NULL;
    node->sibling = NULL;
    push(node);
    return node;
}

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
        createTerminalASTNode(ASTNodeType_TRUE, "true", currentToken.sourceLineNumber);
        readNT();
    }
    else if (isCurrentToken(RESERVED, "false"))
    { // R -> 'false' => 'false'
        createTerminalASTNode(ASTNodeType_FALSE, "false", currentToken.sourceLineNumber);
        readNT();
    }
    else if (isCurrentToken(RESERVED, "nil"))
    { // R -> 'nil' => 'nil'
        createTerminalASTNode(ASTNodeType_NIL, "nil", currentToken.sourceLineNumber);
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
        createTerminalASTNode(ASTNodeType_DUMMY, "dummy", currentToken.sourceLineNumber);
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

void procDB()
{
    printf("procDB\n");

    if (isCurrentTokenType(L_PAREN))
    { // Db -> '(' D ')'
        readNT();
        procD();
        readNT();
        if (!isCurrentTokenType(R_PAREN))
        {
            printf("DB: ')' expected\n"); // Replace with appropriate error handling
        }
        readNT();
    }
    else if (isCurrentTokenType(IDENTIFIER))
    {
        readNT();
        if (isCurrentToken(OPERATOR, ","))
        { // Db -> Vl '=' E => '='
            readNT();
            procVB(); // extra readNT in procVB()
            // VL makes its COMMA nodes for all the tokens EXCEPT the ones
            // we just read above (i.e., the first identifier and the comma after it)
            // Hence, we must pop the top of the tree VL just made and put it under a
            // comma node with the identifier it missed.
            if (!isCurrentToken(OPERATOR, "="))
            {
                printf("DB: = expected.\n"); // Replace with appropriate error handling
            }
            buildNAryASTNode(ASTNodeType_COMMA, 2);
            readNT();
            procE(); // extra readNT in procE()
            buildNAryASTNode(ASTNodeType_EQUAL, 2);
        }
        else
        { // Db -> '<IDENTIFIER>' Vb+ '=' E => 'fcn_form'
            if (isCurrentToken(OPERATOR, "="))
            { // Db -> Vl '=' E => '='; if Vl had only one IDENTIFIER (no commas)
                readNT();
                procE(); // extra readNT in procE()
                buildNAryASTNode(ASTNodeType_EQUAL, 2);
            }
            else
            { // Db -> '<IDENTIFIER>' Vb+ '=' E => 'fcn_form'
                int treesToPop = 0;

                while (isCurrentTokenType(IDENTIFIER) || isCurrentTokenType(L_PAREN))
                {
                    procVB(); // extra readNT in procVB()
                    treesToPop++;
                }

                if (treesToPop == 0)
                {
                    printf("E: at least one 'Vb' expected\n"); // Replace with appropriate error handling
                }

                if (!isCurrentToken(OPERATOR, "="))
                {
                    printf("DB: = expected.\n"); // Replace with appropriate error handling
                }

                readNT();
                procE(); // extra readNT in procE()

                buildNAryASTNode(ASTNodeType_FCNFORM, treesToPop + 2); // +1 for the last E and +1 for the first identifier
            }
        }
    }
}

void procVB()
{
    printf("procVB\n");

    if (isCurrentTokenType(IDENTIFIER))
    { // Vb -> '<IDENTIFIER>'
        readNT();
    }
    else if (isCurrentTokenType(L_PAREN))
    {
        readNT();
        if (isCurrentTokenType(R_PAREN))
        { // Vb -> '(' ')' => '()'
            createTerminalASTNode(ASTNodeType_PAREN, "", currentToken.sourceLineNumber);
            readNT();
        }
        else
        {             // Vb -> '(' Vl ')'
            procVL(); // extra readNT in procVB()
            if (!isCurrentTokenType(R_PAREN))
            {
                printf("VB: ')' expected\n"); // Replace with appropriate error handling
            }
            readNT();
        }
    }
}

void procVL()
{
    printf("procVL\n");

    if (!isCurrentTokenType(IDENTIFIER))
    {
        printf("VL: Identifier expected\n"); // Replace with appropriate error handling
    }
    else
    {
        readNT();
        int treesToPop = 0;
        while (isCurrentToken(OPERATOR, ","))
        { // Vl -> '<IDENTIFIER>' list ',' => ','?;
            readNT();
            if (!isCurrentTokenType(IDENTIFIER))
            {
                printf("VL: Identifier expected\n"); // Replace with appropriate error handling
            }
            readNT();
            treesToPop++;
        }
        if (treesToPop > 0)
        {
            buildNAryASTNode(ASTNodeType_COMMA, treesToPop + 1); // +1 for the first identifier
        }
    }
}