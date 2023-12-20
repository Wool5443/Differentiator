#ifndef DIFF_TREE_DSL_HPP
#define DIFF_TREE_DSL_HPP

#define ERR_DUMP_RET(tree)                                              \
do                                                                      \
{                                                                       \
    ErrorCode _verifyError = tree->Verify();                            \
    if (_verifyError)                                                   \
    {                                                                   \
        tree->Dump();                                                   \
        return _verifyError;                                            \
    }                                                                   \
} while (0);

#define ERR_DUMP_RET_RESULT(tree, poison)                               \
do                                                                      \
{                                                                       \
    ErrorCode _verifyError = tree->Verify();                            \
    if (_verifyError)                                                   \
    {                                                                   \
        tree->Dump();                                                   \
        return { poison, _verifyError };                                \
    }                                                                   \
} while (0);

#define CREATE_NODE(name, val, left, right)                             \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = TreeNode::New(val, left, right);         \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
} while (0)

#define COPY_NODE(name, node)                                           \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = node->Copy();                            \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
} while (0)

#define CREATE_NUMBER(name, val)                                        \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = TreeNode::New({}, nullptr, nullptr);     \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
    name->value.type = NUMBER_TYPE;                                     \
    name->value.value.number = val;                                     \
} while (0)

#define CREATE_OPERATION(name, op, left, right)                         \
TreeNode* name = nullptr;                                               \
do                                                                      \
{                                                                       \
    TreeNodeResult _tempNode = TreeNode::New({}, left, right);          \
    RETURN_ERROR(_tempNode.error);                                      \
    name = _tempNode.value;                                             \
    name->value.type = OPERATION_TYPE;                                  \
    name->value.value.operation = op;                                   \
} while (0)

#define NODE_TYPE(node) ((node)->value.type)
#define NODE_NUMBER(node) ((node)->value.value.number)
#define NODE_OPERATION(node) ((node)->value.value.operation)
#define NODE_VAR(node) ((node)->value.value.var)

#endif
