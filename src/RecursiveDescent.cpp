#include <ctype.h>
#include <string.h>
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

TreeNodeResult _getId(const char** context);

TreeNodeResult _getD(const char** context);

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

    const char* const oldString = CUR_CHAR_PTR;

    char* endPtr = nullptr;

    double val = strtof(CUR_CHAR_PTR, &endPtr);
    CUR_CHAR_PTR = endPtr;

    TreeNodeResult nodeRes = TreeNode::New({}, nullptr, nullptr);
    RETURN_ERROR_RESULT(nodeRes, nullptr);
    TreeNode* node = nodeRes.value;

    SyntaxAssertResult(oldString != CUR_CHAR_PTR, nullptr);

    node->value.type = NUMBER_TYPE;
    node->value.value.number = val;

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

    TreeNodeResult resultRes = _getD(context);
    RETURN_ERROR_RESULT(resultRes, nullptr);
    TreeNode* result = resultRes.value;

    SKIP_SPACES();

    while (*CUR_CHAR_PTR == '*' || *CUR_CHAR_PTR == '/')
    {
        char operation = *CUR_CHAR_PTR;

        CUR_CHAR_PTR++;

        TreeNodeResult valRes = _getD(context);
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
    if (isalpha(*CUR_CHAR_PTR))
        return _getId(context);

    return _getN(context);
}

TreeNodeResult _getId(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

#define DEF_FUNC(name, hasTwoArgs, string, length, ...)                 \
SKIP_SPACES();                                                          \
if (hasTwoArgs && strncasecmp(CUR_CHAR_PTR, string, length) == 0)       \
{                                                                       \
    SKIP_ALPHA();                                                       \
    SKIP_SPACES();                                                      \
                                                                        \
    SyntaxAssertResult(*CUR_CHAR_PTR == '(', nullptr);                  \
    CUR_CHAR_PTR++;                                                     \
                                                                        \
    TreeNodeResult valRes = _getE(context);                             \
    RETURN_ERROR_RESULT(valRes, nullptr);                               \
    TreeNode* val = valRes.value;                                       \
                                                                        \
    SKIP_SPACES();                                                      \
                                                                        \
    SyntaxAssertResult(*CUR_CHAR_PTR == ')', nullptr);                  \
    CUR_CHAR_PTR++;                                                     \
                                                                        \
    TreeNodeResult resultRes = TreeNode::New({}, val, nullptr);         \
    RETURN_ERROR_RESULT(resultRes, nullptr);                            \
    TreeNode* result = resultRes.value;                                 \
                                                                        \
    result->value.type = OPERATION_TYPE;                                \
    result->value.value.operation = name;                               \
    return resultRes;                                                   \
}

#include "DiffFunctions.hpp"

#undef DEF_FUNC

    SyntaxAssertResult(isalpha(*CUR_CHAR_PTR), nullptr);

    TreeNodeResult varRes = TreeNode::New({}, nullptr, nullptr);
    RETURN_ERROR_RESULT(varRes, nullptr);
    TreeNode* var = varRes.value;

    var->value.type = VARIABLE_TYPE;
    var->value.value.var = *CUR_CHAR_PTR;

    CUR_CHAR_PTR++;

    return varRes;
}

TreeNodeResult _getD(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    TreeNodeResult resultRes = _getP(context);
    RETURN_ERROR_RESULT(resultRes, nullptr);
    TreeNode* result = resultRes.value;

    SKIP_SPACES();

    while (*CUR_CHAR_PTR == '^')
    {
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

        result->value.value.operation = POWER_OPERATION;

        SKIP_SPACES();
    }

    return { result, EVERYTHING_FINE };
}
