#include <stdio.h>
#include <math.h>
#include "Differentiator.hpp"
#include "DiffTreeDSL.hpp"
#include "LatexWriter.hpp"

enum Direction
{
    LEFT,
    RIGHT,
};

EvalResult _recEval(TreeNode* node, double var);

ErrorCode _recDiff(TreeNode* node, FILE* texFile);
ErrorCode _diffMultiply(TreeNode* node, FILE* texFile);
ErrorCode _diffDivide(TreeNode* node, FILE* texFile);
ErrorCode _diffPower(TreeNode* node, FILE* texFile);
ErrorCode _diffPowerNumber(TreeNode* node, FILE* texFile);
ErrorCode _diffPowerVar(TreeNode* node, FILE* texFile);
ErrorCode _diffSin(TreeNode* node, FILE* texFile);
ErrorCode _diffCos(TreeNode* node, FILE* texFile);
ErrorCode _diffTan(TreeNode* node, FILE* texFile);
ErrorCode _diffArcsin(TreeNode* node, FILE* texFile);
ErrorCode _diffArccos(TreeNode* node, FILE* texFile);
ErrorCode _diffArctan(TreeNode* node, FILE* texFile);
ErrorCode _diffExp(TreeNode* node, FILE* texFile);
ErrorCode _diffLn(TreeNode* node, FILE* texFile);

ErrorCode _recOptimise(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _recOptimizeConsts(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _recOptimizeNeutrals(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _deleteUnnededAndReplace(TreeNode* toReplace, Direction deleteDirection);

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

    double result = {};
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

ErrorCode Differentiate(Tree* tree, FILE* texFile)
{
    MyAssertSoft(tree, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);
    ERR_DUMP_RET(tree);

    return _recDiff(tree->root, texFile);
}

ErrorCode _recDiff(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
            fprintf(texFile, "Видно, что $(%lg)' = 0$\n\\newline\n", NODE_NUMBER(node));
            NODE_NUMBER(node) = 0;
            return EVERYTHING_FINE;
        case VARIABLE_TYPE:
            fprintf(texFile, "Видно, что $(%c)' = 1$\n\\newline\n", NODE_VAR(node));
            NODE_TYPE(node) = NUMBER_TYPE;
            NODE_NUMBER(node) = 1;
            return EVERYTHING_FINE;
        case OPERATION_TYPE:
        {
            switch (NODE_OPERATION(node))
            {

                #define DEF_FUNC(name, hasOneArg, string, length, code, ...)            \
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

// (uv)' = u'v + uv'
ErrorCode _diffMultiply(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left,  texFile));

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->right, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->right, texFile));

    // u'v
    CREATE_OPERATION(duv, MUL_OPERATION, node->left, v);

    // uv'
    CREATE_OPERATION(udv, MUL_OPERATION, u, node->right);

    RETURN_ERROR(node->SetLeft(duv));
    RETURN_ERROR(node->SetRight(udv));

    NODE_OPERATION(node) = ADD_OPERATION;

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (u / v)' = (u'v - uv') / (v ^ 2)
ErrorCode _diffDivide(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left,  texFile));

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->right, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->right, texFile));

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

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

ErrorCode _diffPower(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    switch (NODE_TYPE(node->right))
    {
        case NUMBER_TYPE:
            return _diffPowerNumber(node, texFile);
        case VARIABLE_TYPE:
        case OPERATION_TYPE:
            return _diffPowerVar(node, texFile);
        default:
            return ERROR_SYNTAX;
    }

    return EVERYTHING_FINE;
}

// (u ^ a)' = a * u ^ (a - 1) * u'
ErrorCode _diffPowerNumber(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left,  texFile));

    // (a - 1)
    CREATE_NUMBER(aMinusOne, node->right->value.value.number - 1);

    // u ^ (a - 1)
    CREATE_OPERATION(uPowAminusOne, POWER_OPERATION, u, aMinusOne);

    // a * u ^ (a - 1)
    CREATE_OPERATION(aMulUPowMinusOne, MUL_OPERATION, node->right, uPowAminusOne);

    RETURN_ERROR(node->SetRight(aMulUPowMinusOne));

    NODE_OPERATION(node) = MUL_OPERATION;

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (u ^ v)' = (e ^ (v * lnu))' = u ^ v * (v * lnu)'
ErrorCode _diffPowerVar(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    CREATE_NODE(uPowV, node->value, node->left, node->right);

    COPY_NODE(u, node->left);
    COPY_NODE(v, node->right);

    CREATE_OPERATION(lnu, LN_OPERATION, u, nullptr);

    CREATE_OPERATION(vlnu, MUL_OPERATION, v, lnu);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(vlnu, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(vlnu, texFile));

    RETURN_ERROR(node->SetLeft(uPowV));
    RETURN_ERROR(node->SetRight(vlnu));

    NODE_OPERATION(node) = MUL_OPERATION;

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (sinu)' = u' * cosu
ErrorCode _diffSin(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left, texFile));

    CREATE_OPERATION(cosu, COS_OPERATION, u, nullptr);

    NODE_OPERATION(node) = MUL_OPERATION;

    RETURN_ERROR(node->SetRight(cosu));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (cosu)' = u' * (-1 * sinu)
ErrorCode _diffCos(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left, texFile));

    CREATE_OPERATION(sinu, SIN_OPERATION, u, nullptr);

    CREATE_NUMBER(neg1, -1);

    CREATE_OPERATION(minusSinu, MUL_OPERATION, neg1, sinu);

    NODE_OPERATION(node) = MUL_OPERATION;

    RETURN_ERROR(node->SetRight(minusSinu));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (tanu)' = u' / (cosu)^2
ErrorCode _diffTan(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left, texFile));

    CREATE_OPERATION(cosu, COS_OPERATION, u, nullptr);

    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(cosuSqr, POWER_OPERATION, cosu, two);

    NODE_OPERATION(node) = DIV_OPERATION;

    RETURN_ERROR(node->SetRight(cosuSqr));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (arcsinu)' = u' / ((1 - u ^ 2) ^ 0.5)
ErrorCode _diffArcsin(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left, texFile));

    CREATE_NUMBER(two, 2);
    CREATE_NUMBER(zeroFive, 0.5);
    CREATE_NUMBER(one, 1);

    CREATE_OPERATION(uSqr, POWER_OPERATION, u, two);
    CREATE_OPERATION(oneSubUSqr, SUB_OPERATION, one, uSqr);
    CREATE_OPERATION(oneSubUSqrSqrt, POWER_OPERATION, oneSubUSqr, zeroFive);

    NODE_OPERATION(node) = DIV_OPERATION;

    RETURN_ERROR(node->SetRight(oneSubUSqrSqrt));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (arccosu)' = -(arcsinu)'
ErrorCode _diffArccos(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);
    RETURN_ERROR(_diffArcsin(node, texFile));

    CREATE_NODE(arcsin, node->value, node->left, node->right);
    CREATE_NUMBER(neg1, -1);

    RETURN_ERROR(node->SetLeft(neg1));
    RETURN_ERROR(node->SetRight(arcsin));

    NODE_OPERATION(node) = MUL_OPERATION;

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (arctanu)' = u' / (1 + u ^ 2)
ErrorCode _diffArctan(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left, texFile));

    CREATE_NUMBER(one, 1);
    CREATE_NUMBER(two, 2);

    CREATE_OPERATION(uSqr, POWER_OPERATION, u, two);
    CREATE_OPERATION(onePlusuSqr, ADD_OPERATION, one, uSqr);

    NODE_OPERATION(node) = DIV_OPERATION;

    RETURN_ERROR(node->SetRight(onePlusuSqr));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (e ^ u)' = e ^ u * u'
ErrorCode _diffExp(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    CREATE_NODE(expu, node->value, node->left, node->right);

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(u, texFile));

    RETURN_ERROR(node->SetLeft(expu));
    RETURN_ERROR(node->SetRight(u));

    NODE_OPERATION(node) = MUL_OPERATION;

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

// (lnu)' = u' / u
ErrorCode _diffLn(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (!node->left || node->right)
        return ERROR_BAD_TREE;

    COPY_NODE(u, node->left);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left, texFile));

    NODE_OPERATION(node) = DIV_OPERATION;

    RETURN_ERROR(node->SetRight(u));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
}

ErrorCode Optimise(Tree* tree, FILE* texFile)
{
    MyAssertSoft(tree, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    bool keepOptimising = true;

    while (keepOptimising)
        RETURN_ERROR(_recOptimise(tree->root, texFile, &keepOptimising));
    
    return EVERYTHING_FINE;
}

ErrorCode _recOptimise(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    *keepOptimizingPtr = false;

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
        case VARIABLE_TYPE:
            return EVERYTHING_FINE;
        default:
            break;
    }

    RETURN_ERROR(_recOptimizeConsts(node, texFile, keepOptimizingPtr));

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
        case VARIABLE_TYPE:
            return EVERYTHING_FINE;
        default:
            break;
    }

    RETURN_ERROR(_recOptimizeNeutrals(node, texFile, keepOptimizingPtr));

    return EVERYTHING_FINE;
}

ErrorCode _recOptimizeConsts(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (!node->right)
        return _recOptimise(node->left, texFile, keepOptimizingPtr);

    RETURN_ERROR(_recOptimise(node->left, texFile, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, texFile, keepOptimizingPtr));

    if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_TYPE(node->right) == NUMBER_TYPE)
    {
        *keepOptimizingPtr = true;
        NODE_TYPE(node) = NUMBER_TYPE;

        switch (NODE_OPERATION(node))
        {
            case ADD_OPERATION:
                fprintf(texFile, "%s\n\\[", GetRandomMathComment());
                RETURN_ERROR(LatexWrite(node, texFile));
                fprintf(texFile, "=");

                NODE_NUMBER(node) = NODE_NUMBER(node->left) + NODE_NUMBER(node->right);
                break;
            case SUB_OPERATION:
                fprintf(texFile, "%s\n\\[", GetRandomMathComment());
                RETURN_ERROR(LatexWrite(node, texFile));
                fprintf(texFile, "=");

                NODE_NUMBER(node) = NODE_NUMBER(node->left) - NODE_NUMBER(node->right);
                break;
            case MUL_OPERATION:
                fprintf(texFile, "%s\n\\[", GetRandomMathComment());
                RETURN_ERROR(LatexWrite(node, texFile));
                fprintf(texFile, "=");

                NODE_NUMBER(node) = NODE_NUMBER(node->left) * NODE_NUMBER(node->right);
                break;
            case DIV_OPERATION:
                fprintf(texFile, "%s\n\\[", GetRandomMathComment());
                RETURN_ERROR(LatexWrite(node, texFile));
                fprintf(texFile, "=");

                if (NODE_NUMBER(node->right) == 0)
                    return ERROR_ZERO_DIVISION;
                NODE_NUMBER(node) = NODE_NUMBER(node->left) / NODE_NUMBER(node->right);
                break;
            case POWER_OPERATION:
                fprintf(texFile, "%s\n\\[", GetRandomMathComment());
                RETURN_ERROR(LatexWrite(node, texFile));
                fprintf(texFile, "=");

                NODE_NUMBER(node) = pow(NODE_NUMBER(node->left), NODE_NUMBER(node->right));
                break;
            default:
                return EVERYTHING_FINE;
        }

        RETURN_ERROR(node->left->Delete());
        RETURN_ERROR(node->right->Delete());

        RETURN_ERROR(LatexWrite(node, texFile));
        fprintf(texFile, "\\]\n");

        return EVERYTHING_FINE;
    }

    return EVERYTHING_FINE;
}

ErrorCode _recOptimizeNeutrals(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (!node->right)
        return _recOptimise(node->left, texFile, keepOptimizingPtr);

    RETURN_ERROR(_recOptimise(node->left, texFile, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, texFile, keepOptimizingPtr));

    switch (NODE_OPERATION(node))
    {
        case ADD_OPERATION:
            fprintf(texFile, "%s\n\\[", GetRandomMathComment());
            RETURN_ERROR(LatexWrite(node, texFile));
            fprintf(texFile, "=");

            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, LEFT));
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case SUB_OPERATION:
            fprintf(texFile, "%s\n\\[", GetRandomMathComment());
            RETURN_ERROR(LatexWrite(node, texFile));
            fprintf(texFile, "=");

            if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case MUL_OPERATION:
            fprintf(texFile, "%s\n\\[", GetRandomMathComment());
            RETURN_ERROR(LatexWrite(node, texFile));
            fprintf(texFile, "=");

            if (NODE_TYPE(node->left)  == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0) ||
                NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());

                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 0;
                break;
            }
            else if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 1))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, LEFT));
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case DIV_OPERATION:
            fprintf(texFile, "%s\n\\[", GetRandomMathComment());
            RETURN_ERROR(LatexWrite(node, texFile));
            fprintf(texFile, "=");

            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());
                    
                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 0;
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case POWER_OPERATION:
            fprintf(texFile, "%s\n\\[", GetRandomMathComment());
            RETURN_ERROR(LatexWrite(node, texFile));
            fprintf(texFile, "=");

            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());
                    
                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 0;
                break;
            }
            else if (NODE_TYPE(node->left)  == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 1) ||
                NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());
                    
                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 1;
                break;
            }
            if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        default:
            return EVERYTHING_FINE;
    }

    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

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

    newNode->value     = {};
    newNode->left      = nullptr;
    newNode->right     = nullptr;
    newNode->parent    = nullptr;
    newNode->nodeCount = SIZET_POISON;
    newNode->id        = BAD_ID;

    free(newNode);

    return EVERYTHING_FINE;
}
