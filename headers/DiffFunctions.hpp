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
DEF_FUNC(MUL_OPERATION,     "*",      RETURN_ERROR(_diffMultiply(node)); break)
DEF_FUNC(DIV_OPERATION,     "/",      RETURN_ERROR(_diffDivide(node)); break)
DEF_FUNC(POWER_OPERATION,   "^",      RETURN_ERROR(_diffPower(node)); break)
DEF_FUNC(SIN_OPERATION,     "sin",    RETURN_ERROR(_diffSin(node)); break)
DEF_FUNC(COS_OPERATION,     "cos",    RETURN_ERROR(_diffCos(node)); break)
DEF_FUNC(TAN_OPERATION,     "tan",    RETURN_ERROR(_diffTan(node)); break)
DEF_FUNC(ARC_SIN_OPERATION, "arcsin", RETURN_ERROR(_diffArcsin(node)); break)
DEF_FUNC(ARC_COS_OPERATION, "arccos", RETURN_ERROR(_diffArccos(node)); break)
DEF_FUNC(ARC_TAN_OPERATION, "arctan", RETURN_ERROR(_diffArctan(node)); break)
DEF_FUNC(EXP_OPERATION,     "exp",    RETURN_ERROR(_diffExp(node)); break)
DEF_FUNC(LN_OPERATION,      "ln",     RETURN_ERROR(_diffLn(node)); break)
