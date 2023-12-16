// DEF_FUNC(name, hasTwoArgs, string, length, code)

DEF_FUNC(ADD_OPERATION, false, "+", 1,
{
    if (!node->left || !node->right)
        return ERROR_BAD_TREE;
    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));
    return EVERYTHING_FINE;
}
)
DEF_FUNC(SUB_OPERATION, false, "-", 1,
{
    if (!node->left || !node->right)
        return ERROR_BAD_TREE;
    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));
    return EVERYTHING_FINE;
}
)
DEF_FUNC(MUL_OPERATION,     false, "*",      1, return _diffMultiply(node))
DEF_FUNC(DIV_OPERATION,     false, "/",      1, return _diffDivide(node))
DEF_FUNC(POWER_OPERATION,   false, "^",      1, return _diffPower(node))
DEF_FUNC(SIN_OPERATION,     true,  "sin",    3, return _diffSin(node))
DEF_FUNC(COS_OPERATION,     true,  "cos",    3, return _diffCos(node))
DEF_FUNC(TAN_OPERATION,     true,  "tan",    3, return _diffTan(node))
DEF_FUNC(ARC_SIN_OPERATION, true,  "arcsin", 6, return _diffArcsin(node))
DEF_FUNC(ARC_COS_OPERATION, true,  "arccos", 6, return _diffArccos(node))
DEF_FUNC(ARC_TAN_OPERATION, true,  "arctan", 6, return _diffArctan(node))
DEF_FUNC(EXP_OPERATION,     true,  "exp",    3, return _diffExp(node))
DEF_FUNC(LN_OPERATION,      true,  "ln",     2, return _diffLn(node))
