#include <stdio.h>
#include <math.h>
#include "Differentiator.hpp"
#include "DiffTreeDSL.hpp"

#define ERR_DUMP_RET(tree)                                              \
do                                                                      \
{                                                                       \
    ErrorCode _verifyError = tree->Verify();                            \
    if (_verifyError)                                                   \
    {                                                                   \
        tree->Dump();                                                   \
        return _verifyError;                                            \
    }                                                                   \
} while (0);

#define ERR_DUMP_RET_RESULT(tree, poison)                               \
do                                                                      \
{                                                                       \
    ErrorCode _verifyError = tree->Verify();                            \
    if (_verifyError)                                                   \
    {                                                                   \
        tree->Dump();                                                   \
        return { poison, _verifyError };                                \
    }                                                                   \
} while (0);

enum Direction
{
    LEFT,
    RIGHT,
};

ErrorCode _recDiff(TreeNode* node);

ErrorCode _diffMultiply(TreeNode* node);

ErrorCode _diffDivide(TreeNode* node);

ErrorCode _diffPower(TreeNode* node);

ErrorCode _diffPowerNumber(TreeNode* node);

ErrorCode _diffPowerVar(TreeNode* node);

ErrorCode _diffSin(TreeNode* node);

ErrorCode _diffCos(TreeNode* node);

ErrorCode _diffTan(TreeNode* node);

ErrorCode _diffArcsin(TreeNode* node);

ErrorCode _diffArccos(TreeNode* node);

ErrorCode _diffArctan(TreeNode* node);

ErrorCode _diffExp(TreeNode* node);

ErrorCode _diffLn(TreeNode* node);

ErrorCode _recOptimise(TreeNode* node, bool* keepOptimizingPtr);

ErrorCode _recOptimizeConsts(TreeNode* node, bool* keepOptimizingPtr);

ErrorCode _recOptimizeNeutrals(TreeNode* node, bool* keepOptimizingPtr);

ErrorCode _deleteUnnededAndReplace(TreeNode* toReplace, Direction deleteDirection);

void PrintTreeElement(FILE* file, TreeElement* treeEl)
{
    switch (treeEl->type)
    {
    case OPERATION_TYPE:
        switch (treeEl->value.operation)
        {

#define DEF_FUNC(name, hasTwoArgs, string, ...)     \
case name:                                          \
    fprintf(file, "op: %s", string);                \
    break;

#include "DiffFunctions.hpp"

#undef DEF_FUNC

            default:
                fprintf(file, "ERROR VAL");
                break;

        }
        break;
    case NUMBER_TYPE:
        fprintf(file, "num: %lg", treeEl->value.number);
        break;
    case VARIABLE_TYPE:
        fprintf(file, "var: %c", treeEl->value.var);
        break;
    default:
        fprintf(stderr, "ERROR ELEMENT\n");
        break;
    }
}

ErrorCode Differentiate(Tree* tree)
{
    MyAssertSoft(tree, ERROR_NULLPTR);
    ERR_DUMP_RET(tree);

    return _recDiff(tree->root);
}

ErrorCode _recDiff(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
            NODE_NUMBER(node) = 0;
            return EVERYTHING_FINE;
        case VARIABLE_TYPE:
            NODE_TYPE(node) = NUMBER_TYPE;
            NODE_NUMBER(node) = 1;
            return EVERYTHING_FINE;
        case OPERATION_TYPE:
        {
            switch (NODE_OPERATION(node))
            {

#define DEF_FUNC(name, hasTwoArgs, string, length, code, ...)           \
case name:                                                              \
code;

#include "DiffFunctions.hpp"

#undef DEF_FUNC

                default:
                    return ERROR_BAD_TREE;
            }
        }
        default:
            return EVERYTHING_FINE;
    }
}

// (uv)' = u'v + uv'
ErrorCode _diffMultiply(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));

    // u'v
    CREATE_OPERATION(duv, MUL_OPERATION, node->left, v);

    // uv'
    CREATE_OPERATION(udv, MUL_OPERATION, u, node->right);

    RETURN_ERROR(node->SetLeft(duv));
    RETURN_ERROR(node->SetRight(udv));

    NODE_OPERATION(node) = ADD_OPERATION;

    return EVERYTHING_FINE;
}

// (u / v)' = (u'v - uv') / (v ^ 2)
ErrorCode _diffDivide(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));

    // u'v
    CREATE_OPERATION(duv, MUL_OPERATION, node->left, v);

    // uv'
    CREATE_OPERATION(udv, MUL_OPERATION, u, node->right);

    // u'v - uv'
    CREATE_OPERATION(leftSub, SUB_OPERATION, duv, udv);

    // v ^ 2
    COPY_NODE(v2, v);
    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(vSquared, POWER_OPERATION, v2, two);

    RETURN_ERROR(node->SetLeft(leftSub));
    RETURN_ERROR(node->SetRight(vSquared));

    NODE_OPERATION(node) = DIV_OPERATION;

    return EVERYTHING_FINE;
}

ErrorCode _diffPower(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    switch (NODE_TYPE(node->right))
    {
        case NUMBER_TYPE:
            return _diffPowerNumber(node);
        case VARIABLE_TYPE:
        case OPERATION_TYPE:
            return _diffPowerVar(node);
        default:
            return ERROR_SYNTAX;
    }

    return EVERYTHING_FINE;
}

// (u ^ a)' = a * u ^ (a - 1) * u'
ErrorCode _diffPowerNumber(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    // (a - 1)
    CREATE_NUMBER(aMinusOne, node->right->value.value.number - 1);

    // u ^ (a - 1)
    CREATE_OPERATION(uPowAminusOne, POWER_OPERATION, u, aMinusOne);

    // a * u ^ (a - 1)
    CREATE_OPERATION(aMulUPowMinusOne, MUL_OPERATION, node->right, uPowAminusOne);

    RETURN_ERROR(node->SetRight(aMulUPowMinusOne));

    NODE_OPERATION(node) = MUL_OPERATION;

    return EVERYTHING_FINE;
}

// (u ^ v)' = (e ^ (v * lnu))' = u ^ v * (v * lnu)'
ErrorCode _diffPowerVar(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    CREATE_NODE(uPowV, node->value, node->left, node->right);

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    CREATE_OPERATION(lnu, LN_OPERATION, u, nullptr);

    CREATE_OPERATION(vlnu, MUL_OPERATION, v, lnu);

    RETURN_ERROR(_recDiff(vlnu));

    RETURN_ERROR(node->SetLeft(uPowV));
    RETURN_ERROR(node->SetRight(vlnu));

    NODE_OPERATION(node) = MUL_OPERATION;

    return EVERYTHING_FINE;
}

// (sinu)' = u' * cosu
ErrorCode _diffSin(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    CREATE_OPERATION(cosu, COS_OPERATION, u, nullptr);

    NODE_OPERATION(node) = MUL_OPERATION;

    return node->SetRight(cosu);
}

// (cosu)' = u' * (-1 * sinu)
ErrorCode _diffCos(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    CREATE_OPERATION(sinu, SIN_OPERATION, u, nullptr);

    CREATE_NUMBER(neg1, -1);

    CREATE_OPERATION(minusSinu, MUL_OPERATION, neg1, sinu);

    NODE_OPERATION(node) = MUL_OPERATION;

    return node->SetRight(minusSinu);
}

// (tanu)' = u' / (cosu)^2
ErrorCode _diffTan(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    CREATE_OPERATION(cosu, COS_OPERATION, u, nullptr);

    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(cosuSqr, POWER_OPERATION, cosu, two);

    NODE_OPERATION(node) = DIV_OPERATION;

    return node->SetRight(cosuSqr);
}

// (arcsinu)' = u' / ((1 - u ^ 2) ^ 0.5)
ErrorCode _diffArcsin(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    COPY_NODE(u, node->left);
    RETURN_ERROR(_recDiff(node->left));

    CREATE_NUMBER(two,  2);
    CREATE_NUMBER(zeroFive, 0.5);
    CREATE_NUMBER(one,  1);

    CREATE_OPERATION(uSqr, POWER_OPERATION, u, two);
    CREATE_OPERATION(oneSubUSqr, SUB_OPERATION, one, uSqr);
    CREATE_OPERATION(oneSubUSqrSqrt, POWER_OPERATION, oneSubUSqr, zeroFive);

    NODE_OPERATION(node) = DIV_OPERATION;

    return node->SetRight(oneSubUSqrSqrt);
}

// (arccosu)' = -(arcsinu)'
ErrorCode _diffArccos(TreeNode* node)
{
    RETURN_ERROR(_diffArcsin(node));

    CREATE_NODE(arcsin, node->value, node->left, node->right);
    CREATE_NUMBER(neg1, -1);

    RETURN_ERROR(node->SetLeft(neg1));
    RETURN_ERROR(node->SetRight(arcsin));

    NODE_OPERATION(node) = MUL_OPERATION;

    return EVERYTHING_FINE;
}

// (arctanu)' = u' / (1 + u ^ 2)
ErrorCode _diffArctan(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    COPY_NODE(u, node->left);
    RETURN_ERROR(_recDiff(node->left));

    CREATE_NUMBER(one, 1);
    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(uSqr, POWER_OPERATION, u, two);
    CREATE_OPERATION(onePlusuSqr, ADD_OPERATION, one, uSqr);

    NODE_OPERATION(node) = DIV_OPERATION;

    return node->SetRight(onePlusuSqr);
}

// (e ^ u)' = e ^ u * u'
ErrorCode _diffExp(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    CREATE_NODE(expu, node->value, node->left, node->right);

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(u));

    RETURN_ERROR(node->SetLeft(expu));
    RETURN_ERROR(node->SetRight(u));

    NODE_OPERATION(node) = MUL_OPERATION;

    return EVERYTHING_FINE;
}

// (lnu)' = u' / u
ErrorCode _diffLn(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    RETURN_ERROR(_recDiff(node->left));

    NODE_OPERATION(node) = DIV_OPERATION;

    return node->SetRight(u);
}

ErrorCode Optimise(Tree* tree)
{
    MyAssertSoft(tree, ERROR_NULLPTR);

    bool keepOptimising = true;

    while (keepOptimising)
        RETURN_ERROR(_recOptimise(tree->root, &keepOptimising));
    
    return EVERYTHING_FINE;
}

ErrorCode _recOptimise(TreeNode* node, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    *keepOptimizingPtr = false;

    RETURN_ERROR(_recOptimizeConsts(node, keepOptimizingPtr));
    RETURN_ERROR(_recOptimizeNeutrals(node, keepOptimizingPtr));

    return EVERYTHING_FINE;
}

ErrorCode _recOptimizeConsts(TreeNode* node, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (NODE_TYPE(node) == NUMBER_TYPE)
        return EVERYTHING_FINE;

    if (!node->left || !node->right)
        return EVERYTHING_FINE;

    RETURN_ERROR(_recOptimise(node->left, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, keepOptimizingPtr));

    if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_TYPE(node->right) == NUMBER_TYPE)
    {
        *keepOptimizingPtr = true;
        NODE_TYPE(node) = NUMBER_TYPE;

        switch (NODE_OPERATION(node))
        {
            case ADD_OPERATION:
                NODE_NUMBER(node) = NODE_NUMBER(node->left) + NODE_NUMBER(node->right);
                break;
            case SUB_OPERATION:
                NODE_NUMBER(node) = NODE_NUMBER(node->left) - NODE_NUMBER(node->right);
                break;
            case MUL_OPERATION:
                NODE_NUMBER(node) = NODE_NUMBER(node->left) * NODE_NUMBER(node->right);
                break;
            case DIV_OPERATION:
                if (NODE_NUMBER(node->right) == 0)
                    return ERROR_ZERO_DIVISION;
                NODE_NUMBER(node) = NODE_NUMBER(node->left) / NODE_NUMBER(node->right);
                break;
            case POWER_OPERATION:
                NODE_NUMBER(node) = pow(NODE_NUMBER(node->left), NODE_NUMBER(node->right));
                break;
            default:
                return EVERYTHING_FINE;
        }

        RETURN_ERROR(node->left->Delete());
        RETURN_ERROR(node->right->Delete());
        return EVERYTHING_FINE;
    }

    return EVERYTHING_FINE;
}

ErrorCode _recOptimizeNeutrals(TreeNode* node, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (NODE_TYPE(node) != OPERATION_TYPE)
        return EVERYTHING_FINE;

    if (!node->left || !node->right)
        return EVERYTHING_FINE;

    RETURN_ERROR(_recOptimise(node->left, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, keepOptimizingPtr));

    switch (NODE_OPERATION(node))
    {
        case ADD_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_NUMBER(node->left) == 0)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, LEFT);
            }
            if (NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 0)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, RIGHT);
            }
            break;
        case SUB_OPERATION:
            if (NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 0)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, RIGHT);
            }
            break;
        case MUL_OPERATION:
            if (NODE_TYPE(node->left)  == NUMBER_TYPE && NODE_NUMBER(node->left)  == 0 ||
                NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 0)
            {
                *keepOptimizingPtr = true;
                CREATE_NUMBER(zero, 0);
                RETURN_ERROR(node->Delete());
                node = zero;

                return EVERYTHING_FINE;
            }
            if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_NUMBER(node->left) == 1)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, LEFT);
            }
            if (NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 1)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, RIGHT);
            }
            break;
        case DIV_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_NUMBER(node->left) == 0)
            {
                *keepOptimizingPtr = true;
                CREATE_NUMBER(zero, 0);
                RETURN_ERROR(node->Delete());
                node = zero;
                
                return EVERYTHING_FINE;
            }
            if (NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 1)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, RIGHT);
            }
            break;
        case POWER_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_NUMBER(node->left) == 0)
            {
                *keepOptimizingPtr = true;
                CREATE_NUMBER(zero, 0);
                RETURN_ERROR(node->Delete());
                node = zero;
                
                return EVERYTHING_FINE;
            }
            if (NODE_TYPE(node->left)  == NUMBER_TYPE && NODE_NUMBER(node->left)  == 1 ||
                NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 0)
            {
                *keepOptimizingPtr = true;
                CREATE_NUMBER(one, 1);
                RETURN_ERROR(node->Delete());
                node = one;
                
                return EVERYTHING_FINE;
            }
            if (NODE_TYPE(node->right) == NUMBER_TYPE && NODE_NUMBER(node->right) == 1)
            {
                *keepOptimizingPtr = true;
                return _deleteUnnededAndReplace(node, RIGHT);
            }
            break;
        default:
            return EVERYTHING_FINE;
    }
    return EVERYTHING_FINE;
}

ErrorCode _deleteUnnededAndReplace(TreeNode* toReplace, Direction deleteDirection)
{
    MyAssertSoft(toReplace, ERROR_NULLPTR);

    TreeNode* newNode = nullptr;

    switch (deleteDirection)
    {
        case LEFT:
            RETURN_ERROR(toReplace->left->Delete());
            newNode = toReplace->right;
            break;
        case RIGHT:
            RETURN_ERROR(toReplace->right->Delete());
            newNode = toReplace->left;
            break;
        default:
            return ERROR_BAD_VALUE;
    }

    toReplace->value = newNode->value;
    toReplace->SetLeft(newNode->left);
    toReplace->SetRight(newNode->right);

    #ifdef SIZE_VERIFICATION
    toReplace->nodeCount = newNode->nodeCount;
    if (toReplace->parent)
        if (toReplace->parent->left == toReplace)
            toReplace->parent->SetLeft(toReplace);
        else if (toReplace->parent->right == toReplace)
            toReplace->parent->SetRight(toReplace);
    #endif

    newNode->value     = {};
    newNode->left      = nullptr;
    newNode->right     = nullptr;
    newNode->parent    = nullptr;
    newNode->nodeCount = SIZET_POISON;
    newNode->id        = BAD_ID;

    free(newNode);

    return EVERYTHING_FINE;
}
