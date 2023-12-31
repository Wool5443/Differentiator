#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

#include "Utils.hpp"
#include "Tree.hpp"

struct EvalResult
{
    double value;
    ErrorCode error;
};

EvalResult Evaluate(Tree* tree, double var);

TreeResult Differentiate(Tree* tree, FILE* texFile);

#endif
