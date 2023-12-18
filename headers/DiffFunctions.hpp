// DEF_FUNC(name, hasOneArg, string, length, code)

DEF_FUNC(ADD_OPERATION, false, "+", 1,
{
    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left,  texFile));

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->right, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->right, texFile));
    
    RETURN_ERROR(node->SetLeft(node->left));
    RETURN_ERROR(node->SetRight(node->right));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");

    return EVERYTHING_FINE;
})
DEF_FUNC(SUB_OPERATION, false, "-", 1,
{
    if (!node->left || !node->right)
        return ERROR_BAD_TREE;

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->left, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->left,  texFile));

    fprintf(texFile, "Необходимо найти $(");
    LatexWrite(node->right, texFile);
    fprintf(texFile, ")'$\n\\newline\n");
    RETURN_ERROR(_recDiff(node->right, texFile));
    
    RETURN_ERROR(node->SetLeft(node->left));
    RETURN_ERROR(node->SetRight(node->right));

    fprintf(texFile, "%s\n\\newline\n", GetRandomMathComment());
    fprintf(texFile, "\\[");
    RETURN_ERROR(LatexWrite(node, texFile));
    fprintf(texFile, "\\]\n");
    
    return EVERYTHING_FINE;
})
DEF_FUNC(MUL_OPERATION,     false, "*",      1, return _diffMultiply(node, texFile))
DEF_FUNC(DIV_OPERATION,     false, "/",      1, return _diffDivide(node, texFile))
DEF_FUNC(POWER_OPERATION,   false, "^",      1, return _diffPower(node, texFile))
DEF_FUNC(SIN_OPERATION,     true,  "sin",    3, return _diffSin(node, texFile))
DEF_FUNC(COS_OPERATION,     true,  "cos",    3, return _diffCos(node, texFile))
DEF_FUNC(TAN_OPERATION,     true,  "tan",    3, return _diffTan(node, texFile))
DEF_FUNC(ARC_SIN_OPERATION, true,  "arcsin", 6, return _diffArcsin(node, texFile))
DEF_FUNC(ARC_COS_OPERATION, true,  "arccos", 6, return _diffArccos(node, texFile))
DEF_FUNC(ARC_TAN_OPERATION, true,  "arctan", 6, return _diffArctan(node, texFile))
DEF_FUNC(EXP_OPERATION,     true,  "exp",    3, return _diffExp(node, texFile))
DEF_FUNC(LN_OPERATION,      true,  "ln",     2, return _diffLn(node, texFile))
