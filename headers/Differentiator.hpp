#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

#include "Utils.hpp"
#include "Tree.hpp"

void PrintTreeElement(FILE* file, TreeElement* treeEl);

ErrorCode Differentiate(Tree* tree);

#endif
