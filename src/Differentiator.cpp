#include <stdio.h>
#include <math.h>
#include "Differentiator.hpp"
#include "DiffTreeDSL.hpp"
#include "LatexWriter.hpp"

EvalResult _recEval(TreeNode* node, double var);

ErrorCode _recDiff(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffAddSub(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffMultiply(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffDivide(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffPower(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffPowerNumber(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffPowerVar(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffSin(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffCos(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffTan(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffArcsin(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffArccos(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffArctan(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffExp(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _diffLn(TreeNode* node, TreeNode* oldNode, FILE* texFile);

ErrorCode _writeNeedToFindDerivative(TreeNode* node, FILE* texFile);
ErrorCode _writeFoundDerivative(TreeNode* node, TreeNode* oldNode, FILE* texFile);

EvalResult Evaluate(Tree* tree, double var)
{
    MyAssertSoftResult(tree, NAN, ERROR_NULLPTR);

    return _recEval(tree->root, var);
}

EvalResult _recEval(TreeNode* node, double var)
{
    MyAssertSoftResult(node, NAN, ERROR_NULLPTR);

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
            return { NODE_NUMBER(node), EVERYTHING_FINE };
        case VARIABLE_TYPE:
            return { var, EVERYTHING_FINE };
        case OPERATION_TYPE:
        default:
            break;
    }

    EvalResult leftRes = {};
    EvalResult rightRes = {};

    if (node->left)
    {
        leftRes = _recEval(node->left, var);
        RETURN_ERROR_RESULT(leftRes, NAN);
    }

    if (node->right)
    {
        rightRes = _recEval(node->right, var);
        RETURN_ERROR_RESULT(rightRes, NAN);
    }

    switch (NODE_OPERATION(node))
    {
        case ADD_OPERATION:
            return { leftRes.value + rightRes.value, EVERYTHING_FINE };
        case SUB_OPERATION:
            return { leftRes.value - rightRes.value, EVERYTHING_FINE };
        case MUL_OPERATION:
            return { leftRes.value * rightRes.value, EVERYTHING_FINE };
        case DIV_OPERATION:
            if (IsEqual(rightRes.value, 0))
                return { NAN, ERROR_ZERO_DIVISION };
            return { leftRes.value / rightRes.value, EVERYTHING_FINE };
        case POWER_OPERATION:
            return { pow(leftRes.value, rightRes.value), EVERYTHING_FINE };
        case SIN_OPERATION:
            return { sin(leftRes.value), EVERYTHING_FINE };
        case COS_OPERATION:
            return { cos(leftRes.value), EVERYTHING_FINE };
        case TAN_OPERATION:
            return { tan(leftRes.value), EVERYTHING_FINE };
        case ARC_SIN_OPERATION:
            return { asin(leftRes.value), EVERYTHING_FINE };
        case ARC_COS_OPERATION:
            return { acos(leftRes.value), EVERYTHING_FINE };
        case ARC_TAN_OPERATION:
            return { atan(leftRes.value), EVERYTHING_FINE };
        case EXP_OPERATION:
            return { exp(leftRes.value), EVERYTHING_FINE };
        case LN_OPERATION:
            return { log(leftRes.value), EVERYTHING_FINE };
        default:
            return { NAN, ERROR_BAD_VALUE };
    }
}

TreeResult Differentiate(Tree* tree, FILE* texFile)
{
    MyAssertSoftResult(tree, {}, ERROR_NULLPTR);
    ERR_DUMP_RET_RESULT(tree, {});

    TreeNodeResult newTreeRootRes = tree->root->Copy();
    RETURN_ERROR_RESULT(newTreeRootRes, {});

    Tree newTree = {};
    ErrorCode error = newTree.Init(newTreeRootRes.value);

    if (error)
        return { {}, error };

    tree->Dump();
    newTree.Dump();

    error = _recDiff(newTree.root, tree->root, texFile);

    if (error)
        return { {}, error };

    #ifdef TEX_WRITE
    fprintf(texFile, "Найдем производную\n\\newline\n\\[");
    error = LatexWrite(tree->root, texFile);
    if (error)
        return { {}, error };
    fprintf(texFile, "\\]\n");
    #endif

    return { newTree, EVERYTHING_FINE };
}

ErrorCode _recDiff(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
            #ifdef TEX_WRITE
            fprintf(texFile, "Видно, что $%lg' = 0$\n\\newline\n", NODE_NUMBER(node));
            #endif
            NODE_NUMBER(node) = 0;
            return EVERYTHING_FINE;
        case VARIABLE_TYPE:
            #ifdef TEX_WRITE
            fprintf(texFile, "Видно, что $%c' = 1$\n\\newline\n", NODE_VAR(node));
            #endif
            NODE_TYPE(node) = NUMBER_TYPE;
            NODE_NUMBER(node) = 1;
            return EVERYTHING_FINE;
        case OPERATION_TYPE:
        {
            switch (NODE_OPERATION(node))
            {

                #define DEF_FUNC(name, priority, hasOneArg, string, length, code, ...)  \
                case name:                                                              \
                code;

                #include "DiffFunctions.hpp"

                #undef DEF_FUNC

                default:
                    return ERROR_BAD_TREE;
            }
        }
        default:
            return ERROR_BAD_VALUE;
    }
}
// (u +- v)' = u' +- v''
ErrorCode _diffAddSub(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    RETURN_ERROR(_writeNeedToFindDerivative(node->left, texFile));
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    RETURN_ERROR(_writeNeedToFindDerivative(node->right, texFile));
    RETURN_ERROR(_recDiff(node->right, oldNode->right, texFile));

    RETURN_ERROR(node->SetLeft(node->left));
    RETURN_ERROR(node->SetRight(node->right));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}


// (uv)' = u'v + uv'
ErrorCode _diffMultiply(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    RETURN_ERROR(_writeNeedToFindDerivative(node->left, texFile));
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    RETURN_ERROR(_writeNeedToFindDerivative(node->right, texFile));
    RETURN_ERROR(_recDiff(node->right, oldNode->right, texFile));

    // u'v
    CREATE_OPERATION(duv, MUL_OPERATION, node->left, v);

    // uv'
    CREATE_OPERATION(udv, MUL_OPERATION, u, node->right);

    RETURN_ERROR(node->SetLeft(duv));
    RETURN_ERROR(node->SetRight(udv));

    NODE_OPERATION(node) = ADD_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (u / v)' = (u'v - uv') / (v ^ 2)
ErrorCode _diffDivide(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    RETURN_ERROR(_writeNeedToFindDerivative(node->left, texFile));
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    RETURN_ERROR(_writeNeedToFindDerivative(node->right, texFile));
    RETURN_ERROR(_recDiff(node->right, oldNode->right, texFile));

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
    UPDATE_PRIORITY(node);

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

ErrorCode _diffPower(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    switch (NODE_TYPE(node->right))
    {
        case NUMBER_TYPE:
            return _diffPowerNumber(node, oldNode, texFile);
        case VARIABLE_TYPE:
        case OPERATION_TYPE:
            return _diffPowerVar(node, oldNode, texFile);
        default:
            return ERROR_SYNTAX;
    }

    return EVERYTHING_FINE;
}

// (u ^ a)' = a * u ^ (a - 1) * u'
ErrorCode _diffPowerNumber(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    RETURN_ERROR(_writeNeedToFindDerivative(node->left, texFile));
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    // (a - 1)
    CREATE_NUMBER(aMinusOne, node->right->value.value.number - 1);

    // u ^ (a - 1)
    CREATE_OPERATION(uPowAminusOne, POWER_OPERATION, u, aMinusOne);

    // a * u ^ (a - 1)
    CREATE_OPERATION(aMulUPowMinusOne, MUL_OPERATION, node->right, uPowAminusOne);

    RETURN_ERROR(node->SetRight(aMulUPowMinusOne));

    NODE_OPERATION(node) = MUL_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (u ^ v)' = (e ^ (v * lnu))' = u ^ v * (v * lnu)'
ErrorCode _diffPowerVar(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    CREATE_NODE(uPowV, node->value, node->left, node->right);

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    CREATE_OPERATION(lnu, LN_OPERATION, u, nullptr);

    CREATE_OPERATION(vlnu, MUL_OPERATION, v, lnu);

    COPY_NODE(vlnuOld, vlnu);

    RETURN_ERROR(_writeNeedToFindDerivative(vlnu, texFile));
    RETURN_ERROR(_recDiff(vlnu, vlnuOld, texFile));

    RETURN_ERROR(vlnuOld->Delete());

    RETURN_ERROR(node->SetLeft(uPowV));
    RETURN_ERROR(node->SetRight(vlnu));

    NODE_OPERATION(node) = MUL_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (sinu)' = u' * cosu
ErrorCode _diffSin(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    CREATE_OPERATION(cosu, COS_OPERATION, u, nullptr);

    NODE_OPERATION(node) = MUL_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(node->SetRight(cosu));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (cosu)' = u' * (-1 * sinu)
ErrorCode _diffCos(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    CREATE_OPERATION(sinu, SIN_OPERATION, u, nullptr);

    CREATE_NUMBER(neg1, -1);

    CREATE_OPERATION(minusSinu, MUL_OPERATION, neg1, sinu);

    NODE_OPERATION(node) = MUL_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(node->SetRight(minusSinu));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (tanu)' = u' / (cosu)^2
ErrorCode _diffTan(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    CREATE_OPERATION(cosu, COS_OPERATION, u, nullptr);

    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(cosuSqr, POWER_OPERATION, cosu, two);

    NODE_OPERATION(node) = DIV_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(node->SetRight(cosuSqr));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (arcsinu)' = u' / ((1 - u ^ 2) ^ 0.5)
ErrorCode _diffArcsin(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    CREATE_NUMBER(two, 2);
    CREATE_NUMBER(zeroFive, 0.5);
    CREATE_NUMBER(one, 1);

    CREATE_OPERATION(uSqr, POWER_OPERATION, u, two);
    CREATE_OPERATION(oneSubUSqr, SUB_OPERATION, one, uSqr);
    CREATE_OPERATION(oneSubUSqrSqrt, POWER_OPERATION, oneSubUSqr, zeroFive);

    NODE_OPERATION(node) = DIV_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(node->SetRight(oneSubUSqrSqrt));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (arccosu)' = -(arcsinu)'
ErrorCode _diffArccos(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    RETURN_ERROR(_diffArcsin(node, oldNode, texFile));

    CREATE_NODE(arcsin, node->value, node->left, node->right);
    CREATE_NUMBER(neg1, -1);

    RETURN_ERROR(node->SetLeft(neg1));
    RETURN_ERROR(node->SetRight(arcsin));

    NODE_OPERATION(node) = MUL_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (arctanu)' = u' / (1 + u ^ 2)
ErrorCode _diffArctan(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    CREATE_NUMBER(one, 1);
    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(uSqr, POWER_OPERATION, u, two);
    CREATE_OPERATION(onePlusuSqr, ADD_OPERATION, one, uSqr);

    NODE_OPERATION(node) = DIV_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(node->SetRight(onePlusuSqr));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (e ^ u)' = e ^ u * u'
ErrorCode _diffExp(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    CREATE_NODE(expu, node->value, node->left, node->right);

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(u, oldNode, texFile));

    RETURN_ERROR(node->SetLeft(expu));
    RETURN_ERROR(node->SetRight(u));

    NODE_OPERATION(node) = MUL_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

// (lnu)' = u' / u
ErrorCode _diffLn(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    _writeNeedToFindDerivative(node->left, texFile);
    RETURN_ERROR(_recDiff(node->left, oldNode->left, texFile));

    NODE_OPERATION(node) = DIV_OPERATION;
    UPDATE_PRIORITY(node);

    RETURN_ERROR(node->SetRight(u));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

ErrorCode _writeNeedToFindDerivative(TreeNode* node, FILE* texFile)
{
    #ifdef TEX_WRITE
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    fprintf(texFile, "Необходимо найти $");
    if (NODE_TYPE(node) == OPERATION_TYPE)
        fprintf(texFile, "(");

    RETURN_ERROR(LatexWrite(node, texFile));

    if (NODE_TYPE(node) == OPERATION_TYPE)
        fprintf(texFile, ")");
    fprintf(texFile, "'$\n\\newline\n");
    #endif

    return EVERYTHING_FINE;
}

ErrorCode _writeFoundDerivative(TreeNode* node, TreeNode* oldNode, FILE* texFile)
{
    #ifdef TEX_WRITE
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(oldNode, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[(");
    RETURN_ERROR(LatexWrite(oldNode, texFile));
    fprintf(texFile, ")' = ");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");
    #endif

    return EVERYTHING_FINE;
}
