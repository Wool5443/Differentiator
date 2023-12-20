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

ErrorCode _recOptimise(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _recOptimizeConsts(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _recOptimizeNeutrals(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _deleteUnnededAndReplace(TreeNode* toReplace, Direction deleteDirection);

ErrorCode _writeNeedToFindDerivative(TreeNode* node, FILE* texFile);
ErrorCode _writeFoundDerivative(TreeNode* node, TreeNode* oldNode, FILE* texFile);
ErrorCode _writeOptimiseStart(TreeNode* node, FILE*texFile);

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

    error = _recDiff(newTree.root, tree->root, texFile);

    if (error)
        return { {}, error };
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
            fprintf(texFile, "Видно, что $(%lg)' = 0$\n\\newline\n", NODE_NUMBER(node));
            #endif
            NODE_NUMBER(node) = 0;
            return EVERYTHING_FINE;
        case VARIABLE_TYPE:
            #ifdef TEX_WRITE
            fprintf(texFile, "Видно, что $(%c)' = 1$\n\\newline\n", NODE_VAR(node));
            #endif
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

    return oldNode->Delete();
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

    RETURN_ERROR(node->SetLeft(uPowV));
    RETURN_ERROR(node->SetRight(vlnu));

    NODE_OPERATION(node) = MUL_OPERATION;

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

    RETURN_ERROR(node->SetRight(u));

    RETURN_ERROR(_writeFoundDerivative(node, oldNode, texFile));

    return EVERYTHING_FINE;
}

ErrorCode Optimise(Tree* tree, FILE* texFile)
{
    MyAssertSoft(tree, ERROR_NULLPTR);
    
    bool keepOptimising = true;

    while (keepOptimising)
        RETURN_ERROR(_recOptimise(tree->root, texFile, &keepOptimising));
    
    return EVERYTHING_FINE;
}

ErrorCode _recOptimise(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    
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
    
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (!node->right)
        return _recOptimise(node->left, texFile, keepOptimizingPtr);

    RETURN_ERROR(_recOptimise(node->left, texFile, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, texFile, keepOptimizingPtr));


    if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_TYPE(node->right) == NUMBER_TYPE)
    {
        *keepOptimizingPtr = true;
        NODE_TYPE(node) = NUMBER_TYPE;

        if (ADD_OPERATION <= NODE_OPERATION(node) && NODE_OPERATION(node) <= POWER_OPERATION)
        {
            RETURN_ERROR(_writeOptimiseStart(node, texFile));
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
    
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (!node->right)
        return _recOptimise(node->left, texFile, keepOptimizingPtr);

    RETURN_ERROR(_recOptimise(node->left, texFile, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, texFile, keepOptimizingPtr));

    bool optimised = false;

    switch (NODE_OPERATION(node))
    {
        case ADD_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, LEFT));
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case SUB_OPERATION:
            if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case MUL_OPERATION:
            if (NODE_TYPE(node->left)  == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0) ||
                NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
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
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, LEFT));
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case DIV_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
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
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case POWER_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
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
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
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
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        default:
            return EVERYTHING_FINE;
    }

    if (optimised)
    {
        RETURN_ERROR(LatexWrite(node, texFile));
        fprintf(texFile, "\\]\n");
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

    newNode->value     = {};
    newNode->left      = nullptr;
    newNode->right     = nullptr;
    newNode->parent    = nullptr;
    newNode->nodeCount = SIZET_POISON;
    newNode->id        = BAD_ID;

    free(newNode);

    return EVERYTHING_FINE;
}

ErrorCode _writeNeedToFindDerivative(TreeNode* node, FILE* texFile)
{
    #ifdef TEX_WRITE
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
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

ErrorCode _writeOptimiseStart(TreeNode* node, FILE* texFile)
{
    #ifdef TEX_WRITE
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    fprintf(texFile, "%s\n\\[", GetRandomMathComment());
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "=");
    #endif

    return EVERYTHING_FINE;
}
