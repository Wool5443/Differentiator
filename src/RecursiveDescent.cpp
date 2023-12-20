#include <ctype.h>
#include <string.h>
#include "RecursiveDescent.hpp"
#include "StringFunctions.hpp"

#define SyntaxAssert(expression, ...)                                   \
do                                                                      \
{                                                                       \
    if (!(expression))                                                  \
    {                                                                   \
        SetConsoleColor(stderr, COLOR_RED);                             \
        fprintf(stderr, "SYNTAX ERROR AT %s\n", CUR_CHAR_PTR);          \
        SetConsoleColor(stderr, COLOR_WHITE);                           \
        __VA_ARGS__;                                                    \
        return ERROR_SYNTAX;                                            \
    }                                                                   \
} while (0)

#define SyntaxAssertResult(expression, poison, ...)                     \
do                                                                      \
{                                                                       \
    if (!(expression))                                                  \
    {                                                                   \
        SetConsoleColor(stderr, COLOR_RED);                             \
        fprintf(stderr, "SYNTAX ERROR AT %s\n", CUR_CHAR_PTR);          \
        SetConsoleColor(stderr, COLOR_WHITE);                           \
        __VA_ARGS__;                                                    \
        return { poison, ERROR_SYNTAX };                                \
    }                                                                   \
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

ErrorCode ParseExpression(Tree* tree, char* string)
{
    MyAssertSoft(tree, ERROR_NULLPTR);
    MyAssertSoft(string, ERROR_NULLPTR);

    const char* filteredSpaces = StringFilter(string, " \t\n", '\0');
    MyAssertSoft(filteredSpaces, ERROR_NULLPTR);

    const char** context = (const char**)&string;

    TreeNodeResult root = _getE(context);

    RETURN_ERROR(root.error);

    SyntaxAssert(*CUR_CHAR_PTR == '\0');

    RETURN_ERROR(tree->Init(root.value));

    return EVERYTHING_FINE;
}

TreeNodeResult _getN(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    const char* const oldString = CUR_CHAR_PTR;

    char* endPtr = nullptr;

    double val = strtof(CUR_CHAR_PTR, &endPtr);
    CUR_CHAR_PTR = endPtr;

    SyntaxAssertResult(oldString != CUR_CHAR_PTR, nullptr);

    TreeNodeResult nodeRes = TreeNode::New({}, nullptr, nullptr);
    RETURN_ERROR_RESULT(nodeRes, nullptr);
    TreeNode* node = nodeRes.value;

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

    while (*CUR_CHAR_PTR == '+' || *CUR_CHAR_PTR == '-')
    {
        char operation = *CUR_CHAR_PTR;

        CUR_CHAR_PTR++;

        TreeNodeResult valRes = _getT(context);
        RETURN_ERROR_RESULT(valRes, nullptr, result->Delete());
        TreeNode* val = valRes.value;

        TreeNodeResult resCopyRes = TreeNode::New(result->value, result->left, result->right);
        RETURN_ERROR_RESULT(resCopyRes, nullptr, result->Delete(); val->Delete());
        TreeNode* resCopy = resCopyRes.value;

        result->nodeCount = 1;

        ErrorCode err = result->SetLeft(resCopy);
        if (err)
        {
            result->Delete();
            val->Delete();
            resCopy->Delete();
            return { nullptr, err };
        }

        err = result->SetRight(val);
        if (err)
        {
            result->Delete();
            val->Delete();
            resCopy->Delete();
            return { nullptr, err };
        }

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
                SyntaxAssertResult(0, nullptr, result->Delete(); val->Delete(); resCopy->Delete());
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

    while (*CUR_CHAR_PTR == '*' || *CUR_CHAR_PTR == '/')
    {
        char operation = *CUR_CHAR_PTR;

        CUR_CHAR_PTR++;

        TreeNodeResult valRes = _getD(context);
        RETURN_ERROR_RESULT(valRes, nullptr, result->Delete());
        TreeNode* val = valRes.value;

        TreeNodeResult resCopyRes = TreeNode::New(result->value, result->left, result->right);
        RETURN_ERROR_RESULT(resCopyRes, nullptr, result->Delete(); val->Delete());
        TreeNode* resCopy = resCopyRes.value;

        result->nodeCount = 1;

        ErrorCode err = result->SetLeft(resCopy);
        if (err)
        {
            result->Delete();
            val->Delete();
            resCopy->Delete();
            return { nullptr, err };
        }

        err = result->SetRight(val);
        if (err)
        {
            result->Delete();
            val->Delete();
            resCopy->Delete();
            return { nullptr, err };
        }

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
                SyntaxAssertResult(0, nullptr, result->Delete(); val->Delete(); resCopy->Delete());
        }
    }

    return { result, EVERYTHING_FINE };
}

TreeNodeResult _getP(const char** context)
{
    MyAssertSoftResult(context, nullptr, ERROR_NULLPTR);

    if (*CUR_CHAR_PTR == '(')
    {
        CUR_CHAR_PTR++;

        TreeNodeResult nodeRes = _getE(context);
        RETURN_ERROR_RESULT(nodeRes, nullptr);

        SyntaxAssertResult(*CUR_CHAR_PTR == ')', nullptr, nodeRes.value->Delete());
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

#define DEF_FUNC(name, priority, hasOneArg, string, length, ...)        \
if (hasOneArg && strncasecmp(CUR_CHAR_PTR, string, length) == 0)        \
{                                                                       \
    CUR_CHAR_PTR += length;                                             \
    SyntaxAssertResult(*CUR_CHAR_PTR == '(', nullptr);                  \
    CUR_CHAR_PTR++;                                                     \
                                                                        \
    TreeNodeResult valRes = _getE(context);                             \
    RETURN_ERROR_RESULT(valRes, nullptr);                               \
    TreeNode* val = valRes.value;                                       \
                                                                        \
    SyntaxAssertResult(*CUR_CHAR_PTR == ')', nullptr, val->Delete());   \
    CUR_CHAR_PTR++;                                                     \
                                                                        \
    TreeNodeResult resultRes = TreeNode::New({}, val, nullptr);         \
    RETURN_ERROR_RESULT(resultRes, nullptr, val->Delete());             \
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

    while (*CUR_CHAR_PTR == '^')
    {
        CUR_CHAR_PTR++;

        TreeNodeResult valRes = _getP(context);
        RETURN_ERROR_RESULT(valRes, nullptr, result->Delete());
        TreeNode* val = valRes.value;

        TreeNodeResult resCopyRes = TreeNode::New(result->value, result->left, result->right);
        RETURN_ERROR_RESULT(resCopyRes, nullptr, result->Delete(), val->Delete());
        TreeNode* resCopy = resCopyRes.value;

        ErrorCode err = result->SetLeft(resCopy);
        if (err)
        {
            result->Delete();
            val->Delete();
            resCopy->Delete();
            return { nullptr, err };
        }

        err = result->SetRight(val);
        if (err)
        {
            result->Delete();
            val->Delete();
            resCopy->Delete();
            return { nullptr, err };
        }

        result->value.type = OPERATION_TYPE;

        result->value.value.operation = POWER_OPERATION;
    }

    return { result, EVERYTHING_FINE };
}
