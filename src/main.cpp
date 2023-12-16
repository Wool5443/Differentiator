#include <stdio.h>
#include "Tree.hpp"
#include "Differentiator.hpp"
#include "RecursiveDescent.hpp"

int main(int argc, const char* const argv[])
{
    MyAssertSoft(argc == 2, ERROR_BAD_FILE);

    Tree::StartHtmlLogging();

    Tree tree = {};
    // tree.Read(argv[1]);

    char* expression = ReadFileToBuf(argv[1]);

    MyAssertSoft(expression, ERROR_NULLPTR);

    ErrorCode error = ParseExpression(&tree, expression);
    MyAssertSoft(!error, error);

    tree.Dump();

    error = Differentiate(&tree);
    MyAssertSoft(!error, error);

    tree.Dump();

    error = Optimise(&tree);
    MyAssertSoft(!error, error);

    tree.Dump();

    tree.Destructor();

    free(expression);

    Tree::EndHtmlLogging();

    return 0;
}
