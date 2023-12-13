#include "Tree.hpp"

int main()
{
    Tree tree = {};
    tree.Read("tree.txt");
    tree.Dump();

    return 0;
}
