#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

enum TreeElementType
{
    OPERATION_TYPE,
    NUMBER_TYPE,
};

struct TreeElement
{
    TreeElementType type;
    union value
    {
        char operation;
        double number;
    };
};

#endif
