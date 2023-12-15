#include <ctype.h>
#include "RecursiveDescent.hpp"

#define SyntaxAssert(expression)                                        \
do                                                                      \
{                                                                       \
    if (!(expression))                                                  \
    {                                                                   \
        SetConsoleColor(stderr, COLOR_RED);                             \
        fprintf(stderr, "SYNTAX ERROR AT %s\n", CUR_CHAR_PTR);          \
        SetConsoleColor(stderr, COLOR_WHITE);                           \
        return ERROR_SYNTAX;                                            \
    }                                                                   \
} while (0)

#define SyntaxAssertResult(expression, poison)                          \
do                                                                      \
{                                                                       \
    if (!(expression))                                                  \
    {                                                                   \
        SetConsoleColor(stderr, COLOR_RED);                             \
        fprintf(stderr, "SYNTAX ERROR AT %s\n", CUR_CHAR_PTR);          \
        SetConsoleColor(stderr, COLOR_WHITE);                           \
        return { poison, ERROR_SYNTAX };                                \
    }                                                                   \
} while (0)

#define SKIP_SPACES()                                                   \
do                                                                      \
{                                                                       \
    while (isspace(*CUR_CHAR_PTR))                                      \
        CUR_CHAR_PTR++;                                                 \
} while (0)

#define SKIP_ALPHA()                                                    \
do                                                                      \
{                                                                       \
    while (isalpha(*CUR_CHAR_PTR))                                      \
        CUR_CHAR_PTR++;                                                 \
} while (0)
#define CUR_CHAR_PTR (*context)

TreeNodeResult _getE(const char** context);

TreeNodeResult _getT(const char** context);

TreeNodeResult _getP(const char** context);

TreeNodeResult _getN(const char** context);

ErrorCode ParseExpression(Tree* tree, const char* string)
{
    MyAssertSoft(tree, ERROR_NULLPTR);
    MyAssertSoft(string, ERROR_NULLPTR);

    const char** context = &string;

    TreeNodeResult root = _getE(context);

    RETURN_ERROR(root.error);

    SKIP_SPACES();
    SyntaxAssert(*CUR_CHAR_PTR == '\0');

    RETURN_ERROR(tree->Init(root.value));

    return EVERYTHING_FINE;
}

TreeNodeResult _getN(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    SKIP_SPACES();

    double val = 0;

    const char* const oldString = CUR_CHAR_PTR;

    while ('0' <= *CUR_CHAR_PTR && *CUR_CHAR_PTR <= '9')
    {
        val = 10 * val + *CUR_CHAR_PTR - '0';
        CUR_CHAR_PTR++;
    }

    TreeNodeResult nodeRes = TreeNode::New({}, nullptr, nullptr);
    RETURN_ERROR_RESULT(nodeRes, nullptr);
    TreeNode* node = nodeRes.value;

    if (oldString != CUR_CHAR_PTR)
    {
        node->value.type = NUMBER_TYPE;
        node->value.value.number = val;
    }
    else
    {
        node->value.type = VARIABLE_TYPE;
        node->value.value.var = CUR_CHAR_PTR;

        SKIP_ALPHA();
        *(char*)CUR_CHAR_PTR = '\0';
        CUR_CHAR_PTR++;
    }

    return { node, EVERYTHING_FINE };
}

TreeNodeResult _getE(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    TreeNodeResult resultRes = _getT(context);
    RETURN_ERROR_RESULT(resultRes, nullptr);
    TreeNode* result = resultRes.value;

    SKIP_SPACES();

    while (*CUR_CHAR_PTR == '+' || *CUR_CHAR_PTR == '-')
    {
        char operation = *CUR_CHAR_PTR;

        CUR_CHAR_PTR++;

        TreeNodeResult valRes = _getT(context);
        RETURN_ERROR_RESULT(valRes, nullptr);
        TreeNode* val = valRes.value;

        TreeNodeResult resCopyRes = TreeNode::New(result->value, result->left, result->right);
        RETURN_ERROR_RESULT(resCopyRes, nullptr);
        TreeNode* resCopy = resCopyRes.value;

        result->nodeCount = 1;

        ErrorCode err = result->SetLeft(resCopy);
        if (err)
            return { nullptr, err };

        err = result->SetRight(val);
        if (err)
            return { nullptr, err };

        result->value.type = OPERATION_TYPE;

        switch (operation)
        {
            case '+':
                result->value.value.operation = ADD_OPERATION;
                break;
            case '-':
                result->value.value.operation = SUB_OPERATION;
                break;
            default:
                SyntaxAssertResult(0, nullptr);
        }
    }

    return { result, EVERYTHING_FINE };
}

TreeNodeResult _getT(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    TreeNodeResult resultRes = _getP(context);
    RETURN_ERROR_RESULT(resultRes, nullptr);
    TreeNode* result = resultRes.value;

    SKIP_SPACES();

    while (*CUR_CHAR_PTR == '*' || *CUR_CHAR_PTR == '/')
    {
        char operation = *CUR_CHAR_PTR;

        CUR_CHAR_PTR++;

        TreeNodeResult valRes = _getP(context);
        RETURN_ERROR_RESULT(valRes, nullptr);
        TreeNode* val = valRes.value;

        TreeNodeResult resCopyRes = TreeNode::New(result->value, result->left, result->right);
        RETURN_ERROR_RESULT(resCopyRes, nullptr);
        TreeNode* resCopy = resCopyRes.value;

        ErrorCode err = result->SetLeft(resCopy);
        if (err)
            return { nullptr, err };

        err = result->SetRight(val);
        if (err)
            return { nullptr, err };

        result->value.type = OPERATION_TYPE;

        switch (operation)
        {
            case '*':
                result->value.value.operation = MUL_OPERATION;
                break;
            case '/':
                result->value.value.operation = DIV_OPERATION;
                break;
            default:
                SyntaxAssertResult(0, nullptr);
        }

        SKIP_SPACES();
    }

    return { result, EVERYTHING_FINE };
}

TreeNodeResult _getP(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    SKIP_SPACES();

    if (*CUR_CHAR_PTR == '(')
    {
        CUR_CHAR_PTR++;

        TreeNodeResult nodeRes = _getE(context);
        RETURN_ERROR_RESULT(nodeRes, nullptr);

        SKIP_SPACES();

        SyntaxAssertResult(*CUR_CHAR_PTR == ')', nullptr);
        CUR_CHAR_PTR++;

        return nodeRes;
    }

    return _getN(context);
}
