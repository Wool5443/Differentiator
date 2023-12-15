#include "Tree.hpp"
#include "Differentiator.hpp"

int main()
{
    Tree tree = {};
    tree.Read("expx.txt");
    tree.Dump();

    Differentiate(&tree);

    tree.Dump();

    return 0;
}
