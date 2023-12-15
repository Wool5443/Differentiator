#include <stdio.h>
#include "Differentiator.hpp"

#define ERR_DUMP_RET(tree)                                          \
do                                                                  \
{                                                                   \
    ErrorCode _verifyError = tree->Verify();                        \
    if (_verifyError)                                               \
    {                                                               \
        tree->Dump();                                               \
        return _verifyError;                                        \
    }                                                               \
} while (0);

#define ERR_DUMP_RET_RESULT(tree, poison)                           \
do                                                                  \
{                                                                   \
    ErrorCode _verifyError = tree->Verify();                        \
    if (_verifyError)                                               \
    {                                                               \
        tree->Dump();                                               \
        return { poison, _verifyError };                            \
    }                                                               \
} while (0);

#define CREATE_NODE(name, val, left, right)                         \
TreeNode* name = nullptr;                                           \
do                                                                  \
{                                                                   \
    TreeNodeResult _tempNode = TreeNode::New(val, left, right);     \
    RETURN_ERROR(_tempNode.error);                                  \
    name = _tempNode.value;                                         \
} while (0)

#define COPY_NODE(name, node)                                       \
TreeNode* name = nullptr;                                           \
do                                                                  \
{                                                                   \
    TreeNodeResult _tempNode = node->Copy();                        \
    RETURN_ERROR(_tempNode.error);                                  \
    name = _tempNode.value;                                         \
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

ErrorCode _diffExp(TreeNode* node);

void PrintTreeElement(FILE* file, TreeElement* treeEl)
{
    switch (treeEl->type)
    {
    case OPERATION_TYPE:
        switch (treeEl->value.operation)
        {
            case LN_OPERATION:
                fprintf(file, "op: ln");
                break;
            case SIN_OPERATION:
                fprintf(file, "op: sin");
                break;
            case COS_OPERATION:
                fprintf(file, "op: cos");
                break;
            case TAN_OPERATION:
                fprintf(file, "op: tan");
                break;
            case EXP_OPERATION:
                fprintf(file, "op: exp");
                break;
            default:
                fprintf(file, "op: %c", treeEl->value.operation);
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
                case '+':
                case '-':
                    if (!node->left || !node->right)
                        return ERROR_BAD_TREE;
                    RETURN_ERROR(_recDiff(node->left));
                    RETURN_ERROR(_recDiff(node->right));
                    return EVERYTHING_FINE;
                case '*':
                    return _diffMultiply(node);
                case '/':
                    return _diffDivide(node);
                case '^':
                    return _diffPower(node);
                case LN_OPERATION:
                    return _diffLn(node);
                case SIN_OPERATION:
                    return _diffSin(node);
                case COS_OPERATION:
                    return _diffCos(node);
                case TAN_OPERATION:
                    return _diffTan(node);
                case EXP_OPERATION:
                    return _diffExp(node);
                default:
                    return ERROR_SYNTAX;
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
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == '*',
                 ERROR_BAD_VALUE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(uCopy,  node->left);
    COPY_NODE(vCopy, node->right);

    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));

    // u'v
    CREATE_NODE(duv, {}, node->left, vCopy);

    duv->value.type = OPERATION_TYPE;
    duv->value.value.operation = '*';

    // uv'
    CREATE_NODE(udv, {}, uCopy, node->right);

    udv->value.type = OPERATION_TYPE;
    udv->value.value.operation = '*';

    RETURN_ERROR(node->SetLeft(duv));
    RETURN_ERROR(node->SetRight(udv));

    node->value.value.operation = '+';

    return EVERYTHING_FINE;
}

// (u / v)' = (u'v - uv') / (v * v)
ErrorCode _diffDivide(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == '/',
                 ERROR_BAD_VALUE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(uCopy, node->left);
    COPY_NODE(vCopy, node->right);

    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));

    // u'v
    CREATE_NODE(duv, {}, node->left, vCopy);

    duv->value.type = OPERATION_TYPE;
    duv->value.value.operation = '*';

    // uv'
    CREATE_NODE(udv, {}, uCopy, node->right);

    udv->value.type = OPERATION_TYPE;
    udv->value.value.operation = '*';

    // u'v - uv'
    CREATE_NODE(leftSub, {}, duv, udv);

    leftSub->value.type = OPERATION_TYPE;
    leftSub->value.value.operation = '-';

    // v ^ 2
    COPY_NODE(rightCopy2, vCopy);
    CREATE_NODE(nodeNumber2, {}, nullptr, nullptr);

    nodeNumber2->value.type = NUMBER_TYPE;
    nodeNumber2->value.value.number = 2;

    CREATE_NODE(vSquared, {}, rightCopy2, nodeNumber2);

    vSquared->value.type = OPERATION_TYPE;
    vSquared->value.value.operation = '^';

    RETURN_ERROR(node->SetLeft(leftSub));
    RETURN_ERROR(node->SetRight(vSquared));

    node->value.value.operation = '/';

    return EVERYTHING_FINE;
}

ErrorCode _diffPower(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == '^',
                 ERROR_BAD_VALUE);

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
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == '^',
                 ERROR_BAD_VALUE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(uCopy, node->left);

    RETURN_ERROR(_recDiff(node->left));

    // (a - 1)
    CREATE_NODE(nodeAsub1, {}, nullptr, nullptr);

    nodeAsub1->value.type = NUMBER_TYPE;
    nodeAsub1->value.value.number = node->right->value.value.number - 1;

    // u ^ (a - 1)
    CREATE_NODE(uPowAsub1, {}, uCopy, nodeAsub1);

    uPowAsub1->value.type = OPERATION_TYPE;
    uPowAsub1->value.value.operation = '^';

    // a * u ^ (a - 1)
    CREATE_NODE(aMulUPowSub1, {}, node->right, uPowAsub1);

    aMulUPowSub1->value.type = OPERATION_TYPE;
    aMulUPowSub1->value.value.operation = '*';

    RETURN_ERROR(node->SetRight(aMulUPowSub1));

    node->value.value.operation = '*';

    return EVERYTHING_FINE;
}

// (u ^ v)' = (e ^ (v * lnu))' = u ^ v * (v * lnu)'
ErrorCode _diffPowerVar(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == '^',
                 ERROR_BAD_VALUE);

    CREATE_NODE(uPowV, node->value, node->left, node->right);

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    CREATE_NODE(lnu, {}, u, nullptr);

    lnu->value.type = OPERATION_TYPE;
    lnu->value.value.operation = LN_OPERATION;

    CREATE_NODE(vlnu, {}, v, lnu);

    vlnu->value.type = OPERATION_TYPE;
    vlnu->value.value.operation = '*';

    RETURN_ERROR(_recDiff(vlnu));

    node->value.value.operation = '*';

    RETURN_ERROR(node->SetLeft(uPowV));
    RETURN_ERROR(node->SetRight(vlnu));

    return EVERYTHING_FINE;
}

// (lnu)' = u' / u
ErrorCode _diffLn(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == LN_OPERATION,
                 ERROR_BAD_VALUE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    RETURN_ERROR(_recDiff(node->left));

    node->value.value.operation = '/';

    return node->SetRight(u);
}

// (sinu)' = cosu * u'
ErrorCode _diffSin(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == SIN_OPERATION,
                 ERROR_BAD_VALUE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    CREATE_NODE(cosu, {}, u, nullptr);
    cosu->value.type = OPERATION_TYPE;
    cosu->value.value.operation = COS_OPERATION;

    node->value.value.operation = '*';

    return node->SetRight(cosu);
}

ErrorCode _diffCos(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == COS_OPERATION,
                 ERROR_BAD_VALUE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    CREATE_NODE(sinu, {}, u, nullptr);
    sinu->value.type = OPERATION_TYPE;
    sinu->value.value.operation = SIN_OPERATION;

    CREATE_NODE(neg1, {}, nullptr, nullptr);
    neg1->value.type = NUMBER_TYPE;
    neg1->value.value.number = -1;

    CREATE_NODE(minusSinu, {}, neg1, sinu);
    minusSinu->value.type = OPERATION_TYPE;
    minusSinu->value.value.operation = '*';

    node->value.value.operation = '*';

    return node->SetRight(minusSinu);
}

// (tanu)' = u' / (cosu)^2
ErrorCode _diffTan(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == TAN_OPERATION,
                 ERROR_BAD_VALUE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(node->left));

    CREATE_NODE(cosu, {}, u, nullptr);
    cosu->value.type = OPERATION_TYPE;
    cosu->value.value.operation = COS_OPERATION;

    CREATE_NODE(node2, {}, nullptr, nullptr);
    node2->value.type = NUMBER_TYPE;
    node2->value.value.number = 2;

    CREATE_NODE(cosuSqr, {}, cosu, node2);
    cosuSqr->value.type = OPERATION_TYPE;
    cosuSqr->value.value.operation = '^';

    node->value.value.operation = '/';

    return node->SetRight(cosuSqr);
}

// (e ^ u)' = e ^ u * u'
ErrorCode _diffExp(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(node->value.type == OPERATION_TYPE && node->value.value.operation == EXP_OPERATION,
                 ERROR_BAD_VALUE);

    CREATE_NODE(expu, node->value, node->left, node->right);

    COPY_NODE(u, node->left);

    RETURN_ERROR(_recDiff(u));

    RETURN_ERROR(node->SetLeft(expu));
    RETURN_ERROR(node->SetRight(u));

    node->value.value.operation = '*';

    return EVERYTHING_FINE;
}
