#include <stdio.h>
#include "Tree.hpp"
#include "Differentiator.hpp"
#include "RecursiveDescent.hpp"
#include "LatexWriter.hpp"
#include "Optimiser.hpp"

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

    #ifdef TEX_WRITE
    fprintf(texFile, "Упростим\n\\newline\n\\[");
    RETURN_ERROR(LatexWrite(tree.root, texFile));
    fprintf(texFile, "\\]\n");
    #endif

    error = Optimise(&tree, texFile);
    MyAssertSoft(!error, error);
    tree.Dump();

    #ifdef TEX_WRITE
    fprintf(texFile, "Найдем производную\n\\newline\n\\[");
    RETURN_ERROR(LatexWrite(tree.root, texFile));
    fprintf(texFile, "\\]\n");
    #endif

    TreeResult treeDiff1Res = Differentiate(&tree, texFile);
    MyAssertSoft(!treeDiff1Res.error, treeDiff1Res.error);
    Tree treeDiff1 = treeDiff1Res.value;
    treeDiff1.Dump();

    #ifdef TEX_WRITE
    fprintf(texFile, "Упростим\n\\newline\n\\[");
    RETURN_ERROR(LatexWrite(treeDiff1.root, texFile));
    fprintf(texFile, "\\]\n");
    #endif

    error = Optimise(&treeDiff1, texFile);
    MyAssertSoft(!error, error);
    tree.Dump();

    Tree::EndHtmlLogging();

    #ifdef TEX_WRITE
    fprintf(texFile, "В итоге имеем\n\\newline\n\\[");
    RETURN_ERROR(LatexWrite(treeDiff1.root, texFile));
    fprintf(texFile, "\\]\n");
    #endif

    error = tree.Destructor();
    MyAssertSoft(!error, error);

    error = treeDiff1.Destructor();
    MyAssertSoft(!error, error);
    free(expression);

    #ifdef TEX_WRITE
    return LatexFileEnd(texFile, "tex");
    #endif

    return 0;
}
