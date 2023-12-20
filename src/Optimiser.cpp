#include "Optimiser.hpp"
#include "LatexWriter.hpp"
#include "DiffTreeDSL.hpp"

enum Direction
{
    LEFT,
    RIGHT,
};

ErrorCode _recOptimise(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _recOptimizeConsts(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _recOptimizeNeutrals(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr);
ErrorCode _deleteUnnededAndReplace(TreeNode* toReplace, Direction deleteDirection);

ErrorCode _writeOptimiseStart(TreeNode* node, FILE*texFile);

ErrorCode Optimise(Tree* tree, FILE* texFile)
{
    MyAssertSoft(tree, ERROR_NULLPTR);

    #ifdef TEX_WRITE
    fprintf(texFile, "Упростим\n\\newline\n\\[");
    RETURN_ERROR(LatexWrite(tree->root, texFile));
    fprintf(texFile, "\\]\n");
    #endif
    
    bool keepOptimising = true;

    while (keepOptimising)
        RETURN_ERROR(_recOptimise(tree->root, texFile, &keepOptimising));
    
    return EVERYTHING_FINE;
}

ErrorCode _recOptimise(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    *keepOptimizingPtr = false;

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
        case VARIABLE_TYPE:
            return EVERYTHING_FINE;
        default:
            break;
    }

    RETURN_ERROR(_recOptimizeConsts(node, texFile, keepOptimizingPtr));

    switch (NODE_TYPE(node))
    {
        case NUMBER_TYPE:
        case VARIABLE_TYPE:
            return EVERYTHING_FINE;
        default:
            break;
    }

    RETURN_ERROR(_recOptimizeNeutrals(node, texFile, keepOptimizingPtr));

    return EVERYTHING_FINE;
}

ErrorCode _recOptimizeConsts(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (!node->right)
        return _recOptimise(node->left, texFile, keepOptimizingPtr);

    RETURN_ERROR(_recOptimise(node->left, texFile, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, texFile, keepOptimizingPtr));

    if (NODE_TYPE(node->left) == NUMBER_TYPE && NODE_TYPE(node->right) == NUMBER_TYPE)
    {
        *keepOptimizingPtr = true;

        RETURN_ERROR(_writeOptimiseStart(node, texFile));

        NODE_TYPE(node) = NUMBER_TYPE;

        switch (NODE_OPERATION(node))
        {
            case ADD_OPERATION:
                NODE_NUMBER(node) = NODE_NUMBER(node->left) + NODE_NUMBER(node->right);
                break;
            case SUB_OPERATION:
                NODE_NUMBER(node) = NODE_NUMBER(node->left) - NODE_NUMBER(node->right);
                break;
            case MUL_OPERATION:
                NODE_NUMBER(node) = NODE_NUMBER(node->left) * NODE_NUMBER(node->right);
                break;
            case DIV_OPERATION:
                if (NODE_NUMBER(node->right) == 0)
                    return ERROR_ZERO_DIVISION;
                NODE_NUMBER(node) = NODE_NUMBER(node->left) / NODE_NUMBER(node->right);
                break;
            case POWER_OPERATION:
                NODE_NUMBER(node) = pow(NODE_NUMBER(node->left), NODE_NUMBER(node->right));
                break;
            default:
                return EVERYTHING_FINE;
        }

        RETURN_ERROR(node->left->Delete());
        RETURN_ERROR(node->right->Delete());

        RETURN_ERROR(LatexWrite(node, texFile));
        fprintf(texFile, "\\]\n");

        return EVERYTHING_FINE;
    }

    return EVERYTHING_FINE;
}

ErrorCode _recOptimizeNeutrals(TreeNode* node, FILE* texFile, bool* keepOptimizingPtr)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    
    MyAssertSoft(keepOptimizingPtr, ERROR_NULLPTR);

    if (!node->right)
        return _recOptimise(node->left, texFile, keepOptimizingPtr);

    RETURN_ERROR(_recOptimise(node->left, texFile, keepOptimizingPtr));
    RETURN_ERROR(_recOptimise(node->right, texFile, keepOptimizingPtr));

    bool optimised = false;

    switch (NODE_OPERATION(node))
    {
        case ADD_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, LEFT));
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case SUB_OPERATION:
            if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case MUL_OPERATION:
            if (NODE_TYPE(node->left)  == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0) ||
                NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());

                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 0;
                break;
            }
            else if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 1))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, LEFT));
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case DIV_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());
                    
                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 0;
                break;
            }
            else if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        case POWER_OPERATION:
            if (NODE_TYPE(node->left) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());
                    
                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 0;
                break;
            }
            else if (NODE_TYPE(node->left)  == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->left), 1) ||
                NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 0))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                if (node->left)
                    RETURN_ERROR(node->left->Delete());
                if (node->right)
                    RETURN_ERROR(node->right->Delete());
                    
                NODE_TYPE(node) = NUMBER_TYPE;
                NODE_NUMBER(node) = 1;
                break;
            }
            if (NODE_TYPE(node->right) == NUMBER_TYPE && IsEqual(NODE_NUMBER(node->right), 1))
            {
                optimised = true;
                RETURN_ERROR(_writeOptimiseStart(node, texFile));
                *keepOptimizingPtr = true;
                RETURN_ERROR(_deleteUnnededAndReplace(node, RIGHT));
                break;
            }
            break;
        default:
            return EVERYTHING_FINE;
    }

    if (optimised)
    {
        RETURN_ERROR(LatexWrite(node, texFile));
        fprintf(texFile, "\\]\n");
    }

    return EVERYTHING_FINE;
}

ErrorCode _deleteUnnededAndReplace(TreeNode* toReplace, Direction deleteDirection)
{
    MyAssertSoft(toReplace, ERROR_NULLPTR);

    TreeNode* newNode = nullptr;

    switch (deleteDirection)
    {
        case LEFT:
            RETURN_ERROR(toReplace->left->Delete());
            newNode = toReplace->right;
            break;
        case RIGHT:
            RETURN_ERROR(toReplace->right->Delete());
            newNode = toReplace->left;
            break;
        default:
            return ERROR_BAD_VALUE;
    }

    toReplace->value = newNode->value;
    toReplace->SetLeft(newNode->left);
    toReplace->SetRight(newNode->right);

    newNode->value     = {};
    newNode->left      = nullptr;
    newNode->right     = nullptr;
    newNode->parent    = nullptr;
    newNode->nodeCount = SIZET_POISON;
    newNode->id        = BAD_ID;

    free(newNode);

    return EVERYTHING_FINE;
}

ErrorCode _writeOptimiseStart(TreeNode* node, FILE* texFile)
{
    #ifdef TEX_WRITE
    MyAssertSoft(node, ERROR_NULLPTR);
    MyAssertSoft(texFile, ERROR_BAD_FILE);

    fprintf(texFile, "%s\n\\[", GetRandomMathComment());
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "=");
    #endif

    return EVERYTHING_FINE;
}
