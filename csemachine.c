#include <stdbool.h>
#include <stdlib.h>

// Assuming you have already implemented the necessary data structures like Stack, ASTNode, Delta, Environment, etc.

#define true 1
#define false 0

typedef enum
{
    ASTNodeType_FALSE,
    ASTNodeType_TRUE,
    ASTNodeType_OR,
    ASTNodeType_TUPLE,
    ASTNodeType_INTEGER,
    ASTNodeType_DELTA,
    ASTNodeType_YSTAR,
    ASTNodeType_ETA,
    ASTNodeType_STRING,
    ASTNodeType_DUMMY
    // Add other node types as needed
} ASTNodeType;

typedef struct
{
    ASTNodeType type;
    char *value;
} ASTNode;

typedef struct
{
    Delta *delta;
} Eta;

typedef struct EnvironmentNode
{
    char *key;
    ASTNode value;
    struct EnvironmentNode *next;
} EnvironmentNode;

// Define Environment struct in C
typedef struct
{
    EnvironmentNode *head;
    struct Environment *parent;
} Environment;

typedef struct
{
    ASTNode **arr;
    int top;
    int capacity;
} Stack;
typedef struct ListNode
{
    char *data;
    struct ListNode *next;
} ListNode;

// Define List struct in C
typedef struct
{
    ListNode *head;
} List;

typedef struct Delta
{
    ASTNodeType type;
    List *boundVars;        // List of strings (char* in C) representing bound variables
    Environment *linkedEnv; // Pointer to Environment representing linked environment
    Stack *body;            // Pointer to Stack (not a standard type in C) of ASTNode pointers
    int index;
} Delta;

typedef struct
{
    Stack *valueStack;
    Delta *rootDelta;
} CSEMachine;

typedef struct EnvironmentBinding
{
    char *identifier;
    ASTNode *value;
    struct EnvironmentBinding *next;
} EnvironmentBinding;

// Define the Environment structure
typedef struct
{
    EnvironmentBinding *head;
} Environment;

// Function prototypes
CSEMachine *createCSEMachine(AST *ast);
void evaluateProgram(CSEMachine *cseMachine);
void processControlStack(CSEMachine *cseMachine, Delta *currentDelta, Environment *currentEnv);
void processCurrentNode(CSEMachine *cseMachine, Delta *currentDelta, Environment *currentEnv, Stack *currentControlStack);

public
class ASTNode
{
private
    ASTNodeType type;
private
    String value;
private
    int sourceLineNumber;
private
    ASTNode child; // Reference to the first child node
private
    ASTNode sibling; // Reference to the next sibling node

    // Constructors, getters, setters, and other methods...

    // Getters and setters for type, value, and sourceLineNumber

public
    ASTNode getChild()
    {
        return child;
    }

public
    void setChild(ASTNode child)
    {
        this.child = child;
    }

public
    ASTNode getSibling()
    {
        return sibling;
    }

public
    void setSibling(ASTNode sibling)
    {
        this.sibling = sibling;
    }
}

public class AST
{
private
    ASTNode root; // The root of the Abstract Syntax Tree

    // Constructors, getters, setters, and other methods...

public
    AST(ASTNode root)
    {
        this.root = root;
    }

public
    ASTNode getRoot()
    {
        return root;
    }

public
    void setRoot(ASTNode root)
    {
        this.root = root;
    }

    // Other methods for manipulating the AST...
}

CSEMachine *
createCSEMachine(AST *ast)
{
    if (!ast->isStandardized())
        exit(EXIT_FAILURE); // Handle the case where AST is not standardized

    CSEMachine *cseMachine = (CSEMachine *)malloc(sizeof(CSEMachine));
    cseMachine->rootDelta = ast->createDeltas();
    cseMachine->rootDelta->linkedEnv = createEnvironment(); // Assuming you have implemented the createEnvironment function
    cseMachine->valueStack = createStack();                 // Assuming you have implemented the createStack function

    return cseMachine;
}

void evaluateProgram(CSEMachine *cseMachine)
{
    processControlStack(cseMachine, cseMachine->rootDelta, cseMachine->rootDelta->linkedEnv);
}

void processControlStack(CSEMachine *cseMachine, Delta *currentDelta, Environment *currentEnv)
{
    Stack *controlStack = createStack(); // Assuming you have implemented the createStack function

    // Add all of the delta's body to the control stack
    ASTNode *node = currentDelta->body;
    while (node != NULL)
    {
        push(controlStack, node); // Assuming you have implemented the push function for the stack
        node = node->sibling;
    }

    while (!isEmpty(controlStack))
    { // Assuming you have implemented the isEmpty function for the stack
        processCurrentNode(cseMachine, currentDelta, currentEnv, controlStack);
    }

    // Don't forget to free the memory allocated for the controlStack
    free(controlStack); // Assuming you have implemented the necessary function to free the stack memory
}

void processCurrentNode(CSEMachine *cseMachine, Delta *currentDelta, Environment *currentEnv, Stack *currentControlStack)
{
    ASTNode *node = pop(currentControlStack); // Assuming you have implemented the pop function for the stack

    if (applyBinaryOperation(node, cseMachine->valueStack)) // Assuming you have implemented the applyBinaryOperation function
        return;
    else if (applyUnaryOperation(node, cseMachine->valueStack)) // Assuming you have implemented the applyUnaryOperation function
        return;
    else
    {
        switch (node->type)
        {
        case IDENTIFIER:
            handleIdentifiers(node, currentEnv, cseMachine->valueStack); // Assuming you have implemented the handleIdentifiers function
            break;
        case NIL:
        case TAU:
            createTuple(node, cseMachine->valueStack); // Assuming you have implemented the createTuple function
            break;
        case BETA:
            handleBeta((Beta *)node, currentControlStack); // Assuming you have implemented the handleBeta function
            break;
        case GAMMA:
            applyGamma(currentDelta, node, currentEnv, currentControlStack, cseMachine->valueStack); // Assuming you have implemented the applyGamma function
            break;
        case DELTA:
            ((Delta *)node)->linkedEnv = currentEnv; // RULE 2
            push(cseMachine->valueStack, node);      // Assuming you have implemented the push function for the stack
            break;
        default:
            // Although we use ASTNodes, a CSEM will only ever see a subset of all possible ASTNodeTypes.
            // These are the types that are NOT standardized away into lambdas and gammas. E.g. types
            // such as LET, WHERE, WITHIN, SIMULTDEF etc will NEVER be encountered by the CSEM
            push(cseMachine->valueStack, node); // Assuming you have implemented the push function for the stack
            break;
        }
    }
}

bool applyBinaryOperation(ASTNode *rator)
{
    switch (rator->type)
    {
    case PLUS:
    case MINUS:
    case MULT:
    case DIV:
    case EXP:
    case LS:
    case LE:
    case GR:
    case GE:
        binaryArithmeticOp(rator->type);
        return true;
    case EQ:
    case NE:
        binaryLogicalEqNeOp(rator->type);
        return true;
    case OR:
    case AND:
        binaryLogicalOrAndOp(rator->type);
        return true;
    case AUG:
        augTuples();
        return true;
    default:
        return false;
    }
}

void binaryArithmeticOp(ASTNodeType type)
{
    ASTNode *rand1 = stack_pop(&valueStack);
    ASTNode *rand2 = stack_pop(&valueStack);

    if (rand1->type != INTEGER || rand2->type != INTEGER)
    {
        EvaluationError_printError(rand1->sourceLineNumber, "Expected two integers; was given \"%s\", \"%s\"", rand1->value, rand2->value);
        // Optionally, handle the error case appropriately (e.g., cleanup and exit the program)
    }

    ASTNode *result = ASTNode_create();
    result->type = INTEGER;

    switch (type)
    {
    case PLUS:
        result->value = intToString(stringToInt(rand1->value) + stringToInt(rand2->value));
        break;
    case MINUS:
        result->value = intToString(stringToInt(rand1->value) - stringToInt(rand2->value));
        break;
    case MULT:
        result->value = intToString(stringToInt(rand1->value) * stringToInt(rand2->value));
        break;
    case DIV:
        result->value = intToString(stringToInt(rand1->value) / stringToInt(rand2->value));
        break;
    case EXP:
        result->value = intToString(pow(stringToInt(rand1->value), stringToInt(rand2->value)));
        break;
    case LS:
        if (stringToInt(rand1->value) < stringToInt(rand2->value))
            pushTrueNode();
        else
            pushFalseNode();
        ASTNode_free(result);
        return;
    case LE:
        if (stringToInt(rand1->value) <= stringToInt(rand2->value))
            pushTrueNode();
        else
            pushFalseNode();
        ASTNode_free(result);
        return;
    case GR:
        if (stringToInt(rand1->value) > stringToInt(rand2->value))
            pushTrueNode();
        else
            pushFalseNode();
        ASTNode_free(result);
        return;
    case GE:
        if (stringToInt(rand1->value) >= stringToInt(rand2->value))
            pushTrueNode();
        else
            pushFalseNode();
        ASTNode_free(result);
        return;
    default:
        // Handle unknown operator case if needed
        break;
    }
    stack_push(&valueStack, result);
}

void binaryLogicalEqNeOp(ASTNodeType type)
{
    ASTNode *rand1 = stack_pop(&valueStack);
    ASTNode *rand2 = stack_pop(&valueStack);

    if (rand1->type == TRUE || rand1->type == FALSE)
    {
        if (rand2->type != TRUE && rand2->type != FALSE)
        {
            EvaluationError_printError(rand1->sourceLineNumber, "Cannot compare dissimilar types; was given \"%s\", \"%s\"", rand1->value, rand2->value);
            // Optionally, handle the error case appropriately (e.g., cleanup and exit the program)
        }
        compareTruthValues(rand1, rand2, type);
        return;
    }

    if (rand1->type != rand2->type)
    {
        EvaluationError_printError(rand1->sourceLineNumber, "Cannot compare dissimilar types; was given \"%s\", \"%s\"", rand1->value, rand2->value);
        // Optionally, handle the error case appropriately (e.g., cleanup and exit the program)
    }

    if (rand1->type == STRING)
    {
        compareStrings(rand1, rand2, type);
    }
    else if (rand1->type == INTEGER)
    {
        compareIntegers(rand1, rand2, type);
    }
    else
    {
        EvaluationError_printError(rand1->sourceLineNumber, "Don't know how to %s \"%s\", \"%s\"", type == EQ ? "compare for equality" : "compare for inequality", rand1->value, rand2->value);
        // Optionally, handle the error case appropriately (e.g., cleanup and exit the program)
    }
}

void compareTruthValues(ASTNode *rand1, ASTNode *rand2, ASTNodeType type)
{
    if (rand1->type == rand2->type)
    {
        if (type == EQ)
            pushTrueNode();
        else
            pushFalseNode();
    }
    else
    {
        if (type == EQ)
            pushFalseNode();
        else
            pushTrueNode();
    }
}

void compareStrings(ASTNode *rand1, ASTNode *rand2, ASTNodeType type)
{
    if (strcmp(rand1->value, rand2->value) == 0)
    {
        if (type == EQ)
            pushTrueNode();
        else
            pushFalseNode();
    }
    else
    {
        if (type == EQ)
            pushFalseNode();
        else
            pushTrueNode();
    }
}

void compareIntegers(ASTNode *rand1, ASTNode *rand2, ASTNodeType type)
{
    int value1 = atoi(rand1->value);
    int value2 = atoi(rand2->value);

    if (value1 == value2)
    {
        if (type == EQ)
            pushTrueNode();
        else
            pushFalseNode();
    }
    else
    {
        if (type == EQ)
            pushFalseNode();
        else
            pushTrueNode();
    }
}
void binaryLogicalOrAndOp(ASTNodeType type)
{
    ASTNode *rand1 = valueStack.pop();
    ASTNode *rand2 = valueStack.pop();

    if ((rand1->type == ASTNodeType_TRUE || rand1->type == ASTNodeType_FALSE) &&
        (rand2->type == ASTNodeType_TRUE || rand2->type == ASTNodeType_FALSE))
    {
        orAndTruthValues(rand1, rand2, type);
        return;
    }

    EvaluationError_printError(rand1->sourceLineNumber, "Don't know how to " + type + " \"" + rand1->value + "\", \"" + rand2->value + "\"");
}

void orAndTruthValues(ASTNode *rand1, ASTNode *rand2, ASTNodeType type)
{
    if (type == ASTNodeType_OR)
    {
        if (rand1->type == ASTNodeType_TRUE || rand2->type == ASTNodeType_TRUE)
            pushTrueNode();
        else
            pushFalseNode();
    }
    else
    {
        if (rand1->type == ASTNodeType_TRUE && rand2->type == ASTNodeType_TRUE)
            pushTrueNode();
        else
            pushFalseNode();
    }
}

void augTuples()
{
    ASTNode *rand1 = valueStack.pop();
    ASTNode *rand2 = valueStack.pop();

    if (rand1->type != ASTNodeType_TUPLE)
        EvaluationError_printError(rand1->sourceLineNumber, "Cannot augment a non-tuple \"" + rand1->value + "\"");

    ASTNode *childNode = rand1->child;
    if (childNode == NULL)
        rand1->child = rand2;
    else
    {
        while (childNode->sibling != NULL)
            childNode = childNode->sibling;
        childNode->sibling = rand2;
    }
    rand2->sibling = NULL;

    valueStack.push(rand1);
}
#include <stdbool.h> // Include this for using boolean data type

bool applyUnaryOperation(ASTNode *rator)
{
    switch (rator->type)
    {
    case ASTNodeType_NOT:
        not();
        return true;
    case ASTNodeType_NEG:
        neg();
        return true;
    default:
        return false;
    }
}

void not()
{
    ASTNode *rand = valueStack.pop();
    if (rand->type != ASTNodeType_TRUE && rand->type != ASTNodeType_FALSE)
        EvaluationError_printError(rand->sourceLineNumber, "Expecting a truthvalue; was given \"" + rand->value + "\"");

    if (rand->type == ASTNodeType_TRUE)
        pushFalseNode();
    else
        pushTrueNode();
}

void neg()
{
    ASTNode *rand = valueStack.pop();
    if (rand->type != ASTNodeType_INTEGER)
        EvaluationError_printError(rand->sourceLineNumber, "Expecting an integer; was given \"" + rand->value + "\"");

    ASTNode *result = (ASTNode *)malloc(sizeof(ASTNode));
    result->type = ASTNodeType_INTEGER;
    int value = atoi(rand->value); // Convert string to integer
    int negValue = -value;
    sprintf(result->value, "%d", negValue); // Convert integer back to string
    valueStack.push(result);
}
#include <stdbool.h> // Include this for using boolean data type

void applyGamma(Delta *currentDelta, ASTNode *node, Environment *currentEnv, Stack *currentControlStack)
{
    ASTNode *rator = valueStack_pop();
    ASTNode *rand = valueStack_pop();

    if (rator->type == ASTNodeType_DELTA)
    {
        Delta *nextDelta = (Delta *)rator;

        // Delta has a link to the environment in effect when it is pushed on to the value stack (search
        // for 'RULE 2' in this file to see where it's done)
        // We construct a new environment here that will contain all the bindings (single or multiple)
        // required by this Delta. This new environment will link back to the environment carried by the Delta.
        Environment *newEnv = (Environment *)malloc(sizeof(Environment));
        newEnv->parent = nextDelta->linkedEnv;

        // RULE 4
        if (nextDelta->boundVarsSize == 1)
        {
            addMapping(newEnv, nextDelta->boundVars[0], rand);
        }
        // RULE 11
        else
        {
            if (rand->type != ASTNodeType_TUPLE)
                EvaluationError_printError(rand->sourceLineNumber, "Expected a tuple; was given \"" + rand->value + "\"");

            for (int i = 0; i < nextDelta->boundVarsSize; i++)
            {
                ASTNode *tupleChild = getNthTupleChild((Tuple *)rand, i + 1); // + 1 coz tuple indexing starts at 1
                addMapping(newEnv, nextDelta->boundVars[i], tupleChild);
            }
        }

        processControlStack(nextDelta, newEnv);
        return;
    }

    else if (rator->type == ASTNodeType_YSTAR)
    {
        // RULE 12
        if (rand->type != ASTNodeType_DELTA)
            EvaluationError_printError(rand->sourceLineNumber, "Expected a Delta; was given \"" + rand->value + "\"");

        Eta *etaNode = (Eta *)malloc(sizeof(Eta));
        etaNode->delta = (Delta *)rand;
        valueStack_push(etaNode);
        return;
    }
    else if (rator->type == ASTNodeType_ETA)
    {
        // RULE 13
        // push back the rand, the eta, and then the delta it contains
        valueStack_push(rand);
        valueStack_push(rator);
        valueStack_push(((Eta *)rator)->delta);
        // push back two gammas (one for the eta and one for the delta)
        currentControlStack_push(currentControlStack, node);
        currentControlStack_push(currentControlStack, node);
        return;
    }
    else if (rator->type == ASTNodeType_TUPLE)
    {
        tupleSelection((Tuple *)rator, rand);
        return;
    }
    else if (evaluateReservedIdentifiers(rator, rand, currentControlStack))
    {
        return;
    }
    else
    {
        EvaluationError_printError(rator->sourceLineNumber, "Don't know how to evaluate \"" + rator->value + "\"");
    }
}
#include <stdbool.h> // Include this for using boolean data type

bool evaluateReservedIdentifiers(ASTNode *rator, ASTNode *rand, Stack *currentControlStack)
{
    const char *ratorValue = rator->value;

    if (strcmp(ratorValue, "Isinteger") == 0)
    {
        checkTypeAndPushTrueOrFalse(rand, ASTNodeType_INTEGER);
        return true;
    }
    else if (strcmp(ratorValue, "Isstring") == 0)
    {
        checkTypeAndPushTrueOrFalse(rand, ASTNodeType_STRING);
        return true;
    }
    else if (strcmp(ratorValue, "Isdummy") == 0)
    {
        checkTypeAndPushTrueOrFalse(rand, ASTNodeType_DUMMY);
        return true;
    }
    else if (strcmp(ratorValue, "Isfunction") == 0)
    {
        checkTypeAndPushTrueOrFalse(rand, ASTNodeType_DELTA);
        return true;
    }
    else if (strcmp(ratorValue, "Istuple") == 0)
    {
        checkTypeAndPushTrueOrFalse(rand, ASTNodeType_TUPLE);
        return true;
    }
    else if (strcmp(ratorValue, "Istruthvalue") == 0)
    {
        if (rand->type == ASTNodeType_TRUE || rand->type == ASTNodeType_FALSE)
            pushTrueNode();
        else
            pushFalseNode();
        return true;
    }
    else if (strcmp(ratorValue, "Stem") == 0)
    {
        stem(rand);
        return true;
    }
    else if (strcmp(ratorValue, "Stern") == 0)
    {
        stern(rand);
        return true;
    }
    else if (strcmp(ratorValue, "Conc") == 0 || strcmp(ratorValue, "conc") == 0)
    { // typos
        conc(rand, currentControlStack);
        return true;
    }
    else if (strcmp(ratorValue, "Print") == 0 || strcmp(ratorValue, "print") == 0)
    { // typos
        printNodeValue(rand);
        pushDummyNode();
        return true;
    }
    else if (strcmp(ratorValue, "ItoS") == 0)
    {
        itos(rand);
        return true;
    }
    else if (strcmp(ratorValue, "Order") == 0)
    {
        order(rand);
        return true;
    }
    else if (strcmp(ratorValue, "Null") == 0)
    {
        isNullTuple(rand);
        return true;
    }
    else
    {
        return false;
    }
}
void checkTypeAndPushTrueOrFalse(ASTNode *rand, ASTNodeType type)
{
    if (rand->type == type)
        pushTrueNode();
    else
        pushFalseNode();
}

void pushTrueNode()
{
    ASTNode *trueNode = createASTNode(ASTNodeType_TRUE, "true");
    valueStackPush(trueNode);
}

void pushFalseNode()
{
    ASTNode *falseNode = createASTNode(ASTNodeType_FALSE, "false");
    valueStackPush(falseNode);
}

void pushDummyNode()
{
    ASTNode *dummyNode = createASTNode(ASTNodeType_DUMMY, NULL);
    valueStackPush(dummyNode);
}

void stem(ASTNode *rand)
{
    if (rand->type != ASTNodeType_STRING)
        EvaluationError_printError(rand->sourceLineNumber, "Expected a string; was given \"%s\"", rand->value);

    if (rand->value == NULL || strlen(rand->value) == 0)
        rand->value = strdup("");
    else
    {
        char *temp = malloc(2 * sizeof(char));
        strncpy(temp, rand->value, 1);
        temp[1] = '\0';
        free(rand->value);
        rand->value = temp;
    }

    valueStackPush(rand);
}

void stern(ASTNode *rand)
{
    if (rand->type != ASTNodeType_STRING)
        EvaluationError_printError(rand->sourceLineNumber, "Expected a string; was given \"%s\"", rand->value);

    if (rand->value == NULL || strlen(rand->value) <= 1)
        rand->value = strdup("");
    else
    {
        char *temp = malloc(strlen(rand->value) * sizeof(char));
        strncpy(temp, rand->value + 1, strlen(rand->value) - 1);
        temp[strlen(rand->value) - 1] = '\0';
        free(rand->value);
        rand->value = temp;
    }

    valueStackPush(rand);
}
void conc(ASTNode *rand1, Stack *currentControlStack)
{
    ASTNode *rand2 = valueStackPop();
    if (rand1->type != ASTNodeType_STRING || rand2->type != ASTNodeType_STRING)
        EvaluationError_printError(rand1->sourceLineNumber, "Expected two strings; was given \"%s\", \"%s\"", rand1->value, rand2->value);

    char *resultValue = malloc((strlen(rand1->value) + strlen(rand2->value) + 1) * sizeof(char));
    strcpy(resultValue, rand1->value);
    strcat(resultValue, rand2->value);

    ASTNode *result = createASTNode(ASTNodeType_STRING, resultValue);
    valueStackPush(result);

    free(resultValue);
}

void itos(ASTNode *rand)
{
    if (rand->type != ASTNodeType_INTEGER)
        EvaluationError_printError(rand->sourceLineNumber, "Expected an integer; was given \"%s\"", rand->value);

    rand->type = ASTNodeType_STRING; // all values are stored internally as strings, so nothing else to do
    valueStackPush(rand);
}

void order(ASTNode *rand)
{
    if (rand->type != ASTNodeType_TUPLE)
        EvaluationError_printError(rand->sourceLineNumber, "Expected a tuple; was given \"%s\"", rand->value);

    int numChildren = getNumChildren(rand);
    char *resultValue = intToString(numChildren);

    ASTNode *result = createASTNode(ASTNodeType_INTEGER, resultValue);
    valueStackPush(result);

    free(resultValue);
}

void isNullTuple(ASTNode *rand)
{
    if (rand->type != ASTNodeType_TUPLE)
        EvaluationError_printError(rand->sourceLineNumber, "Expected a tuple; was given \"%s\"", rand->value);

    if (getNumChildren(rand) == 0)
        pushTrueNode();
    else
        pushFalseNode();
}

// RULE 10
void tupleSelection(Tuple *rator, ASTNode *rand)
{
    if (rand->type != ASTNodeType_INTEGER)
        EvaluationError_printError(rand->sourceLineNumber, "Non-integer tuple selection with \"%s\"", rand->value);

    int n = stringToInt(rand->value);
    ASTNode *result = getNthTupleChild(rator, n);
    if (result == NULL)
        EvaluationError_printError(rand->sourceLineNumber, "Tuple selection index %d out of bounds", n);

    valueStackPush(result);
}

/**
 * Get the nth element of the tuple. Note that n starts from 1 and NOT 0.
 */
ASTNode *getNthTupleChild(Tuple *tupleNode, int n)
{
    ASTNode *childNode = tupleNode->child;
    for (int i = 1; i < n; ++i)
    { // tuple selection index starts at 1
        if (childNode == NULL)
            break;
        childNode = childNode->sibling;
    }
    return childNode;
}

void handleIdentifiers(ASTNode *node, Environment *currentEnv)
{
    ASTNode *value = Environment_lookup(currentEnv, node->value);
    if (value != NULL)
        valueStackPush(value);
    else if (isReservedIdentifier(node->value))
        valueStackPush(node);
    else
        EvaluationError_printError(node->sourceLineNumber, "Undeclared identifier \"%s\"", node->value);
}

// RULE 9
void createTuple(ASTNode *node)
{
    int numChildren = getNumChildren(node);
    Tuple *tupleNode = createTupleNode();

    if (numChildren == 0)
    {
        valueStackPush(tupleNode);
        return;
    }

    ASTNode *childNode = NULL;
    ASTNode *tempNode = NULL;
    for (int i = 0; i < numChildren; ++i)
    {
        if (childNode == NULL)
            childNode = valueStackPop();
        else if (tempNode == NULL)
        {
            tempNode = valueStackPop();
            childNode->sibling = tempNode;
        }
        else
        {
            tempNode->sibling = valueStackPop();
            tempNode = tempNode->sibling;
        }
    }

    tempNode->sibling = NULL;
    tupleNode->child = childNode;
    valueStackPush(tupleNode);
}

// RULE 8
void handleBeta(Beta *node, Stack *currentControlStack)
{
    ASTNode *conditionResultNode = valueStackPop();

    if (conditionResultNode->type != ASTNodeType_TRUE && conditionResultNode->type != ASTNodeType_FALSE)
        EvaluationError_printError(conditionResultNode->sourceLineNumber, "Expecting a truthvalue; found \"%s\"", conditionResultNode->value);

    if (conditionResultNode->type == ASTNodeType_TRUE)
        stackAddAll(currentControlStack, node->thenBody);
    else
        stackAddAll(currentControlStack, node->elseBody);
}

int getNumChildren(ASTNode *node)
{
    int numChildren = 0;
    ASTNode *childNode = node->child;
    while (childNode != NULL)
    {
        numChildren++;
        childNode = childNode->sibling;
    }
    return numChildren;
}

void printNodeValue(ASTNode *rand)
{
    char *evaluationResult = rand->value;
    evaluationResult = replaceSpecialCharacters(evaluationResult);
    printf("%s", evaluationResult);
}

// Note how this list is different from the one defined in Scanner.c
bool isReservedIdentifier(const char *value)
{
    const char *reservedIdentifiers[] = {
        "Isinteger",
        "Isstring",
        "Istuple",
        "Isdummy",
        "Istruthvalue",
        "Isfunction",
        "ItoS",
        "Order",
        "Conc",
        "conc", // typos
        "Stern",
        "Stem",
        "Null",
        "Print",
        "print", // typos
        "neg",
        NULL};

    for (int i = 0; reservedIdentifiers[i] != NULL; ++i)
    {
        if (strcmp(reservedIdentifiers[i], value) == 0)
            return true;
    }

    return false;
};

// --------------------------------- Delta ----------------------------------------

// Function to initialize a new Delta instance
Delta *new_Delta()
{
    Delta *delta = (Delta *)malloc(sizeof(Delta));
    if (delta != NULL)
    {
        delta->type = DELTA;
        delta->boundVars = new_List(); // You need to define and implement the List data structure in C
        delta->linkedEnv = NULL;       // Initialize the linkedEnv to NULL
        delta->body = new_Stack();     // You need to define and implement the Stack data structure in C
        delta->index = 0;              // Initialize the index to a default value
    }
    return delta;
}

// Function to accept a NodeCopier visitor (not shown in the provided Java code)
Delta *Delta_accept(NodeCopier *nodeCopier, Delta *delta)
{
    // Implement the NodeCopier functionality if required
    // ...
    return delta;
}

// Function to get the string representation of the lambda closure
char *Delta_getValue(Delta *delta)
{
    // Implement the function to get the string representation of the lambda closure
    // The code will be similar to the provided Java code for the getValue() method
    // ...
}

// Function to get the list of bound variables
List *Delta_getBoundVars(Delta *delta)
{
    return delta->boundVars;
}

// Function to add a bound variable to the list
void Delta_addBoundVar(Delta *delta, char *boundVar)
{
    // Implement the function to add a bound variable to the list
    // The code will be similar to the provided Java code for the addBoundVars() method
    // ...
}

// Function to set the list of bound variables
void Delta_setBoundVars(Delta *delta, List *boundVars)
{
    delta->boundVars = boundVars;
}

// Function to get the body of the lambda closure
Stack *Delta_getBody(Delta *delta)
{
    return delta->body;
}

// Function to set the body of the lambda closure
void Delta_setBody(Delta *delta, Stack *body)
{
    delta->body = body;
}

// Function to get the index of the lambda closure
int Delta_getIndex(Delta *delta)
{
    return delta->index;
}

// Function to set the index of the lambda closure
void Delta_setIndex(Delta *delta, int index)
{
    delta->index = index;
}

// Function to get the linked environment of the lambda closure
Environment *Delta_getLinkedEnv(Delta *delta)
{
    return delta->linkedEnv;
}

// Function to set the linked environment of the lambda closure
void Delta_setLinkedEnv(Delta *delta, Environment *linkedEnv)
{
    delta->linkedEnv = linkedEnv;
}

// --------------------------------- Environment ----------------------------------------

Environment *new_Environment()
{
    Environment *env = (Environment *)malloc(sizeof(Environment));
    if (env != NULL)
    {
        env->head = NULL;
        env->parent = NULL;
    }
    return env;
}

// Function to add a new mapping to the environment
void addMapping(Environment *env, char *key, ASTNode value)
{
    EnvironmentNode *new_node = (EnvironmentNode *)malloc(sizeof(EnvironmentNode));
    if (new_node != NULL)
    {
        new_node->key = strdup(key); // Make a copy of the key string
        new_node->value = value;
        new_node->next = env->head;
        env->head = new_node;
    }
}

// Function to look up a key in the environment and its parent environments
ASTNode lookup(Environment *env, char *key)
{
    EnvironmentNode *temp = env->head;
    while (temp != NULL)
    {
        if (strcmp(temp->key, key) == 0)
        {
            return temp->value; // Return the found value
        }
        temp = temp->next;
    }

    // If the key is not found in this environment, check the parent environment recursively
    if (env->parent != NULL)
    {
        return lookup(env->parent, key);
    }

    // If the key is not found in any environment, return NULL
    return NULL;
}

// Function to delete the environment and free its memory
void delete_Environment(Environment *env)
{
    while (env->head != NULL)
    {
        EnvironmentNode *temp = env->head;
        env->head = env->head->next;
        free(temp->key);
        // Free any memory used by the ASTNode value, if needed
        // ...
        free(temp);
    }
    free(env);
}

// --------------------------------- Eta ----------------------------------------

Eta *new_Eta()
{
    Eta *eta = (Eta *)malloc(sizeof(Eta));
    if (eta != NULL)
    {
        eta->delta = NULL;
    }
    return eta;
}

// Function to set the Delta reference for the Eta
void setDelta(Eta *eta, Delta *delta)
{
    eta->delta = delta;
}

// Function to delete the Eta and free its memory
void delete_Eta(Eta *eta)
{
    free(eta);
}

// --------------------------------- Evaluate Error ----------------------------------------

void printError(const char *fileName, int sourceLineNumber, const char *message);

// Implementation
void printError(const char *fileName, int sourceLineNumber, const char *message)
{
    printf("%s:%d: %s\n", fileName, sourceLineNumber, message);
    exit(1);
}

// --------------------------------- Node Copier ----------------------------------------

ASTNode *copy_ASTNode(ASTNode *astNode)
{
    ASTNode *copy = (ASTNode *)malloc(sizeof(ASTNode));
    if (astNode->child != NULL)
        copy->child = copy_ASTNode(astNode->child);
    if (astNode->sibling != NULL)
        copy->sibling = copy_ASTNode(astNode->sibling);
    copy->type = astNode->type;
    copy->value = astNode->value;
    copy->sourceLineNumber = astNode->sourceLineNumber;
    return copy;
}

Beta *copy_Beta(Beta *beta)
{
    Beta *copy = (Beta *)malloc(sizeof(Beta));
    if (beta->child != NULL)
        copy->child = copy_ASTNode(beta->child);
    if (beta->sibling != NULL)
        copy->sibling = copy_ASTNode(beta->sibling);
    copy->type = beta->type;
    copy->value = beta->value;
    copy->sourceLineNumber = beta->sourceLineNumber;

    // Copy thenBody
    copy->thenBody = create_empty_stack();
    StackNode *temp = beta->thenBody->top;
    while (temp != NULL)
    {
        ASTNode *tempNode = (ASTNode *)temp->data;
        ASTNode *tempCopy = copy_ASTNode(tempNode);
        push(copy->thenBody, tempCopy);
        temp = temp->next;
    }

    // Copy elseBody
    copy->elseBody = create_empty_stack();
    temp = beta->elseBody->top;
    while (temp != NULL)
    {
        ASTNode *tempNode = (ASTNode *)temp->data;
        ASTNode *tempCopy = copy_ASTNode(tempNode);
        push(copy->elseBody, tempCopy);
        temp = temp->next;
    }

    return copy;
}

Eta *copy_Eta(Eta *eta)
{
    Eta *copy = (Eta *)malloc(sizeof(Eta));
    if (eta->child != NULL)
        copy->child = copy_ASTNode(eta->child);
    if (eta->sibling != NULL)
        copy->sibling = copy_ASTNode(eta->sibling);
    copy->type = eta->type;
    copy->value = eta->value;
    copy->sourceLineNumber = eta->sourceLineNumber;

    copy->delta = copy_Delta(eta->delta);

    return copy;
}

Delta *copy_Delta(Delta *delta)
{
    Delta *copy = (Delta *)malloc(sizeof(Delta));
    if (delta->child != NULL)
        copy->child = copy_ASTNode(delta->child);
    if (delta->sibling != NULL)
        copy->sibling = copy_ASTNode(delta->sibling);
    copy->type = delta->type;
    copy->value = delta->value;
    copy->index = delta->index;
    copy->sourceLineNumber = delta->sourceLineNumber;

    // Copy body
    copy->body = create_empty_stack();
    StackNode *temp = delta->body->top;
    while (temp != NULL)
    {
        ASTNode *tempNode = (ASTNode *)temp->data;
        ASTNode *tempCopy = copy_ASTNode(tempNode);
        push(copy->body, tempCopy);
        temp = temp->next;
    }

    // Copy boundVars
    copy->boundVars = create_empty_list();
    ListNode *tempNode = delta->boundVars->front;
    while (tempNode != NULL)
    {
        const char *boundVar = (const char *)tempNode->data;
        char *boundVarCopy = strdup(boundVar);
        add_to_list(copy->boundVars, boundVarCopy);
        tempNode = tempNode->next;
    }

    copy->linkedEnv = delta->linkedEnv;

    return copy;
}

Tuple *copy_Tuple(Tuple *tuple)
{
    Tuple *copy = (Tuple *)malloc(sizeof(Tuple));
    if (tuple->child != NULL)
        copy->child = copy_ASTNode(tuple->child);
    if (tuple->sibling != NULL)
        copy->sibling = copy_ASTNode(tuple->sibling);
    copy->type = tuple->type;
    copy->value = tuple->value;
    copy->sourceLineNumber = tuple->sourceLineNumber;
    return copy;
}

// --------------------------------- Tuple ----------------------------------------
