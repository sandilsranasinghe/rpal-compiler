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
