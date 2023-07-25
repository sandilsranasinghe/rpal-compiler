#include <stdbool.h>
#include <stdlib.h>

// Assuming you have already implemented the necessary data structures like Stack, ASTNode, Delta, Environment, etc.

typedef struct
{
    Stack *valueStack;
    Delta *rootDelta;
} CSEMachine;

// Function prototypes
CSEMachine *createCSEMachine(AST *ast);
void evaluateProgram(CSEMachine *cseMachine);
void processControlStack(CSEMachine *cseMachine, Delta *currentDelta, Environment *currentEnv);
void processCurrentNode(CSEMachine *cseMachine, Delta *currentDelta, Environment *currentEnv, Stack *currentControlStack);

CSEMachine *createCSEMachine(AST *ast)
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
}
