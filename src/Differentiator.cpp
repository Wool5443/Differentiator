#include <stdio.h>
#include "Differentiator.hpp"

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

#define CREATE_NODE(name, val, left, right)                             \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = TreeNode::New(val, left, right);         \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
} while (0)

#define COPY_NODE(name, node)                                           \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = node->Copy();                            \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
} while (0)

#define CREATE_NUMBER(name, val)                                        \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = TreeNode::New({}, nullptr, nullptr);     \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
    name->value.type = NUMBER_TYPE;                                     \
    name->value.value.number = val;                                     \
} while (0)

#define CREATE_OPERATION(name, op, left, right)                         \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = TreeNode::New({}, left, right);          \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
    name->value.type = OPERATION_TYPE;                                  \
    name->value.value.operation = op;                                   \
} while (0)

ErrorCode _recDiff(TreeNode* node);

ErrorCode _diffMultiply(TreeNode* node);

ErrorCode _diffDivide(TreeNode* node);

ErrorCode _diffPower(TreeNode* node);

ErrorCode _diffPowerNumber(TreeNode* node);

ErrorCode _diffPowerVar(TreeNode* node);

ErrorCode _diffLn(TreeNode* node);

ErrorCode _diffSin(TreeNode* node);

ErrorCode _diffCos(TreeNode* node);

ErrorCode _diffTan(TreeNode* node);

ErrorCode _diffArcsin(TreeNode* node);

ErrorCode _diffArccos(TreeNode* node);

ErrorCode _diffArctan(TreeNode* node);

ErrorCode _diffExp(TreeNode* node);

void PrintTreeElement(FILE* file, TreeElement* treeEl)
{
    switch (treeEl->type)
    {
    case OPERATION_TYPE:
        switch (treeEl->value.operation)
        {

#define DEF_FUNC(name, string, ...)                 \
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
        fprintf(file, "var: %s", treeEl->value.var);
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

    if (tree->root->value.type != OPERATION_TYPE)
        return ERROR_BAD_TREE;

    return _recDiff(tree->root);
}

ErrorCode _recDiff(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    switch (node->value.type)
    {
        case NUMBER_TYPE:
            node->value.value.number = 0;
            return EVERYTHING_FINE;
        case VARIABLE_TYPE:
            node->value.type = NUMBER_TYPE;
            node->value.value.number = 1;
            return EVERYTHING_FINE;
        case OPERATION_TYPE:
        {
            switch (node->value.value.operation)
            {

#define DEF_FUNC(name, string, code, ...)                               \
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

    COPY_NODE(u,  node->left);
    COPY_NODE(v, node->right);

    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));

    // u'v
    CREATE_OPERATION(duv, MUL_OPERATION, node->left, v);

    // uv'
    CREATE_OPERATION(udv, MUL_OPERATION, u, node->right);

    RETURN_ERROR(node->SetLeft(duv));
    RETURN_ERROR(node->SetRight(udv));

    node->value.value.operation = ADD_OPERATION;

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

    node->value.value.operation = DIV_OPERATION;

    return EVERYTHING_FINE;
}

ErrorCode _diffPower(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    switch (node->right->value.type)
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

    node->value.value.operation = MUL_OPERATION;

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

    node->value.value.operation = MUL_OPERATION;

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

    node->value.value.operation = MUL_OPERATION;

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

    node->value.value.operation = MUL_OPERATION;

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

    node->value.value.operation = DIV_OPERATION;

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

    node->value.value.operation = DIV_OPERATION;

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

    node->value.value.operation = MUL_OPERATION;

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

    node->value.value.operation = DIV_OPERATION;

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

    node->value.value.operation = MUL_OPERATION;

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

    node->value.value.operation = DIV_OPERATION;

    return node->SetRight(u);
}
