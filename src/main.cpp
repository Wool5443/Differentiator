#include <stdio.h>
#include "Tree.hpp"
#include "Differentiator.hpp"
#include "RecursiveDescent.hpp"
#include "LatexWriter.hpp"

int main(int argc, const char* const argv[])
{
    MyAssertSoft(argc == 2, ERROR_BAD_FILE);

    Tree::StartHtmlLogging();

    TexFileResult texFileRes = LatexFileInit("tex");
    MyAssertSoft(!texFileRes.error, texFileRes.error);
    FILE* texFile = texFileRes.value;

    Tree tree = {};

    char* expression = ReadFileToBuf(argv[1]);

    MyAssertSoft(expression, ERROR_NULLPTR);

    ErrorCode error = ParseExpression(&tree, expression);
    MyAssertSoft(!error, error);
    tree.Dump();

    fprintf(texFile, "Упростим\n\\[");
    RETURN_ERROR(LatexWrite(tree.root, texFile));
    fprintf(texFile, "\\]\n");

    error = Optimise(&tree, texFile);
    MyAssertSoft(!error, error);
    tree.Dump();

    fprintf(texFile, "Найдем производную\n\\[");
    RETURN_ERROR(LatexWrite(tree.root, texFile));
    fprintf(texFile, "\\]\n");

    error = Differentiate(&tree, texFile);
    MyAssertSoft(!error, error);
    tree.Dump();

    fprintf(texFile, "Упростим\n\\[");
    RETURN_ERROR(LatexWrite(tree.root, texFile));
    fprintf(texFile, "\\]\n");

    error = Optimise(&tree, texFile);
    MyAssertSoft(!error, error);
    tree.Dump();

    Tree::EndHtmlLogging();

    fprintf(texFile, "В итоге имеем\n\\newline\n\\[");
    RETURN_ERROR(LatexWrite(tree.root, texFile));
    fprintf(texFile, "\\]\n");

    error = tree.Destructor();
    MyAssertSoft(!error, error);
    free(expression);

    return LatexFileEnd(texFile, "tex");
}
