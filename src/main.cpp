#include "Tree.hpp"
#include "Differentiator.hpp"

int main(int argc, const char* const argv[])
{
    MyAssertSoft(argc == 2, ERROR_BAD_FILE);

    Tree::StartHtmlLogging();

    Tree tree = {};
    tree.Read(argv[1]);
    tree.Dump();

    Differentiate(&tree);

    tree.Dump();

    Tree::EndHtmlLogging();

    return 0;
}
