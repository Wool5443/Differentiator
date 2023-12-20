#include <stdio.h>
#include <time.h>
#include "DiffTreeDSL.hpp"
#include "FunnyMathComments.hpp"
#include "LatexWriter.hpp"

static const size_t MAX_FILE_LENGTH = 256;
static const size_t MAX_COMMAND_LENGTH = 512;

static const char* TEX_FILE_NAME = "math.tex";

static const char PREAMBLE[] = 
"\\documentclass[12pt, a4paper]{article}\n"
"\\usepackage[T1,T2A]{fontenc}\n"
"\\usepackage[utf8]{inputenc}\n"
"\\usepackage{amsmath,amssymb}\n"
"\\usepackage[russian,english]{babel}\n"
"\\usepackage{wrapfig}\n"
"\\usepackage{graphicx}\n"
"\\usepackage{titlesec}\n"
"\\setlength{\\parindent}{0pt}\n"
"\\begin{document}\n"
"\\begin{center}\n";
static const size_t PREAMBLE_SIZE = sizeof(PREAMBLE) / sizeof(*PREAMBLE);

ErrorCode _recTexWrite(TreeNode* node, FILE* texFile);

ErrorCode _recTexWriteOperation(TreeNode* node, FILE* texFile);

ErrorCode _recTexWriteFunction(TreeNode* node, FILE* texFile, bool hasOneArg, const char* funcName);

TexFileResult LatexFileInit(const char* texFolder)
{
    MyAssertSoftResult(texFolder, nullptr, ERROR_NULLPTR);
    char texFilePath[MAX_FILE_LENGTH] = "";
    sprintf(texFilePath, "%s/%s", texFolder, TEX_FILE_NAME);
    
    srand(time(NULL));

    FILE* texFile = fopen(texFilePath, "w");
    if (!texFile)
        return { nullptr, ERROR_BAD_FILE };

    fprintf(texFile, "%s", PREAMBLE);

    return { texFile, EVERYTHING_FINE };
}

ErrorCode LatexFileEnd(FILE* texFile, const char* texFolder)
{
    MyAssertSoft(texFile, ERROR_NULLPTR);

    fprintf(texFile, "\\end{center}\n\\end{document}\n");
    fclose(texFile);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "pdflatex -output-directory %s %s/%s", texFolder, texFolder, TEX_FILE_NAME);

    system(command);

    return EVERYTHING_FINE;
}

const char* GetRandomMathComment()
{
    return MATH_PHRASES[rand() % NUMBER_OF_PHRASES];
}

ErrorCode LatexWrite(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    RETURN_ERROR(_recTexWrite(node, texFile));

    return EVERYTHING_FINE;
}

ErrorCode _recTexWrite(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
            fprintf(texFile, "{%lg}", NODE_NUMBER(node));
            break;
        case VARIABLE_TYPE:
            fprintf(texFile, "{%c}", NODE_VAR(node));
            break;
        case OPERATION_TYPE:
            return _recTexWriteOperation(node, texFile);
        default:
            return ERROR_BAD_VALUE;
    }

    return EVERYTHING_FINE;
}

ErrorCode _recTexWriteOperation(TreeNode* node, FILE* texFile)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    switch (NODE_OPERATION(node))
    {

#define DEF_FUNC(name, hasOneArg, string, ...)                          \
case name:                                                              \
    return _recTexWriteFunction(node, texFile, hasOneArg, string);

#include "DiffFunctions.hpp"

#undef DEF_FUNC

        default:
            return ERROR_BAD_VALUE;
    }
}

ErrorCode _recTexWriteFunction(TreeNode* node, FILE* texFile, bool hasOneArg, const char* funcName)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    if (hasOneArg)
    {
        if (!node->left || node->right)
            return ERROR_BAD_TREE;

        fprintf(texFile, "\\%s(", funcName);
        RETURN_ERROR(_recTexWrite(node->left, texFile));
        fprintf(texFile, ")");
        
    }
    else
    {
        if (!node->left || !node->right)
            return ERROR_BAD_TREE;

        switch (NODE_OPERATION(node))
        {
            case ADD_OPERATION:
            case SUB_OPERATION:
            case POWER_OPERATION:
                fprintf(texFile, "(");
                RETURN_ERROR(_recTexWrite(node->left, texFile));
                fprintf(texFile, "%s", funcName);
                RETURN_ERROR(_recTexWrite(node->right, texFile));
                fprintf(texFile, ")");
                break;
            case MUL_OPERATION:
                fprintf(texFile, "(");
                RETURN_ERROR(_recTexWrite(node->left, texFile));
                fprintf(texFile, "\\cdot");
                RETURN_ERROR(_recTexWrite(node->right, texFile));
                fprintf(texFile, ")");
                break;
            case DIV_OPERATION:
                fprintf(texFile, "\\frac{");
                RETURN_ERROR(_recTexWrite(node->left, texFile));
                fprintf(texFile, "}{");
                RETURN_ERROR(_recTexWrite(node->right, texFile));
                fprintf(texFile, "}");
                break;
            default:
                return ERROR_BAD_VALUE;
        }

    }

    return EVERYTHING_FINE;
}
