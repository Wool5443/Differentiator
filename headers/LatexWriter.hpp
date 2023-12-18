#ifndef LATEX_WRITER_HPP
#define LATEX_WRITER_HPP

#include "Tree.hpp"

struct TexFileResult
{
    FILE* value;
    ErrorCode error;
};

TexFileResult LatexFileInit(const char* texFolder);

ErrorCode LatexFileEnd(FILE* texFile, const char* texFolder);

ErrorCode LatexWrite(TreeNode* node, FILE* texFile);

const char* GetRandomMathComment();

#endif
