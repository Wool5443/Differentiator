// DEF_FUNC(name, string)

DEF_FUNC(ADD_OPERATION,  "+",
{
    if (!node->left || !node->right)
        return ERROR_BAD_TREE;
    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));
    return EVERYTHING_FINE;
}
)
DEF_FUNC(SUB_OPERATION,  "-",
{
    if (!node->left || !node->right)
        return ERROR_BAD_TREE;
    RETURN_ERROR(_recDiff(node->left));
    RETURN_ERROR(_recDiff(node->right));
    return EVERYTHING_FINE;
}
)
DEF_FUNC(MUL_OPERATION,     "*",      return _diffMultiply(node))
DEF_FUNC(DIV_OPERATION,     "/",      return _diffDivide(node))
DEF_FUNC(POWER_OPERATION,   "^",      return _diffPower(node))
DEF_FUNC(SIN_OPERATION,     "sin",    return _diffSin(node))
DEF_FUNC(COS_OPERATION,     "cos",    return _diffCos(node))
DEF_FUNC(TAN_OPERATION,     "tan",    return _diffTan(node))
DEF_FUNC(ARC_SIN_OPERATION, "arcsin", return _diffArcsin(node))
DEF_FUNC(ARC_COS_OPERATION, "arccos", return _diffArccos(node))
DEF_FUNC(ARC_TAN_OPERATION, "arctan", return _diffArctan(node))
DEF_FUNC(EXP_OPERATION,     "exp",    return _diffExp(node);)
DEF_FUNC(LN_OPERATION,      "ln",     return _diffLn(node);)
