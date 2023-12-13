#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Tree.hpp"
#include "OneginFunctions.hpp"
#include "MinMax.hpp"

static const size_t MAX_PATH_LENGTH = 128;
static const size_t MAX_COMMAND_LENGTH = 256;

static FILE* HTML_FILE = NULL;

static TreeNodeResult _recCopy(TreeNode* node);

#ifdef SIZE_VERIFICATION
static ErrorCode _recUpdateParentNodeCount(TreeNode* node, ssize_t change);
#endif

static TreeNodeCountResult _recCountNodes(TreeNode* node);

#ifdef SIZE_VERIFICATION
ErrorCode _recRecalcNodes(TreeNode* node);
#endif

static ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile,
                                             size_t curDepth, const size_t maxDepth);

static ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile, size_t curDepth, const size_t maxDepth);

static ErrorCode _recPrint(TreeNode* node, FILE* outFile);

static TreeNodeResult _recRead(Text* input, size_t* wordNum);

static TreeNodeResult _recReadOpenBracket(Text* input, const char* openBracket,
                                          size_t* wordNum);

#define ERR_DUMP_RET(tree)                              \
do                                                      \
{                                                       \
    ErrorCode _verifyError = tree->Verify();            \
    if (_verifyError)                                   \
    {                                                   \
        tree->Dump();                                   \
        return _verifyError;                            \
    }                                                   \
} while (0);

#define ERR_DUMP_RET_RESULT(tree, poison)               \
do                                                      \
{                                                       \
    ErrorCode _verifyError = tree->Verify();            \
    if (_verifyError)                                   \
    {                                                   \
        tree->Dump();                                   \
        return { poison, _verifyError };                \
    }                                                   \
} while (0);

TreeNodeResult TreeNode::New(TreeElement_t value, TreeNode* left, TreeNode* right)
{
    static size_t CURRENT_ID = 1;

    TreeNode* node = (TreeNode*)calloc(1, sizeof(TreeNode));
    if (!node)
        return { NULL, ERROR_NO_MEMORY };

    node->value = value;

    #ifdef SIZE_VERIFICATION
    node->nodeCount = 1;
    #endif

    if (left)
    {
        left->parent = node;
        #ifdef SIZE_VERIFICATION
        node->nodeCount += left->nodeCount;
        #endif
    }
    node->left = left;

    if (right)
    {
        right->parent = node;
        #ifdef SIZE_VERIFICATION
        node->nodeCount += right->nodeCount;
        #endif
    }
    node->right = right;

    node->parent = nullptr;

    node->id = CURRENT_ID++;

    return { node, EVERYTHING_FINE };
}

ErrorCode TreeNode::Delete()
{
    if (this->id == BAD_ID)
        return ERROR_TREE_LOOP;

    this->id = BAD_ID;

    if (this->parent)
    {
        if (this->parent->left == this)
        {
            if (this->parent->right == this)
                return ERROR_TREE_LOOP;
            this->parent->left = nullptr;
        }
        else if (this->parent->right == this)
            this->parent->right = nullptr;
        else
            return ERROR_TREE_LOOP;

        #ifdef SIZE_VERIFICATION
        _recUpdateParentNodeCount(this->parent, -(ssize_t)this->nodeCount);
        #endif
    }

    if (this->left)
    {
        if (this->left->parent != this)
            return ERROR_TREE_LOOP;
        RETURN_ERROR(this->left->Delete());
    }
    if (this->right)
    {
        if (this->right->parent != this)
            return ERROR_TREE_LOOP;
        RETURN_ERROR(this->right->Delete());
    }

    this->value  = TREE_POISON;
    this->left   = nullptr;
    this->right  = nullptr;
    this->parent = nullptr;

    #ifdef SIZE_VERIFICATION
    this->nodeCount = SIZET_POISON;
    #endif

    free(this);

    return EVERYTHING_FINE;
}

TreeNodeResult TreeNode::Copy()
{
    if (this->left && this->left->parent != this)
        return { nullptr, ERROR_TREE_LOOP };
    if (this->right && this->right->parent != this)
        return { nullptr, ERROR_TREE_LOOP };

    return _recCopy(this);
}

ErrorCode TreeNode::SetLeft(TreeNode* left)
{
    MyAssertSoft(left, ERROR_NULLPTR);

    this->left = left;
    #ifdef SIZE_VERIFICATION
    this->nodeCount += left->nodeCount;
    #endif
    left->parent = this;

    #ifdef SIZE_VERIFICATION
    if (this->parent)
        return _recUpdateParentNodeCount(this->parent, left->nodeCount);
    #endif

    return EVERYTHING_FINE;
}
ErrorCode TreeNode::SetRight(TreeNode* right)
{
    MyAssertSoft(right, ERROR_NULLPTR);

    this->right = right;
    #ifdef SIZE_VERIFICATION
    this->nodeCount += right->nodeCount;
    #endif
    right->parent = this;

    #ifdef SIZE_VERIFICATION
    if (this->parent)
        return _recUpdateParentNodeCount(this->parent, right->nodeCount);
    #endif

    return EVERYTHING_FINE;
}

static TreeNodeResult _recCopy(TreeNode* node)
{
    MyAssertSoftResult(node, nullptr, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return { nullptr, ERROR_TREE_LOOP };
    
    size_t oldId = node->id;
    node->id = BAD_ID;

    TreeNodeResult leftChild = { nullptr, EVERYTHING_FINE };
    if (node->left)
    {
        if (node->left->parent != node)
            return { nullptr, ERROR_TREE_LOOP };
        leftChild = _recCopy(node->left);
    }
    RETURN_ERROR_RESULT(leftChild, nullptr);

    TreeNodeResult rightChild = { nullptr, EVERYTHING_FINE };

    if (node->right)
    {
        if (node->right->parent != node)
        {
            leftChild.value->Delete();
            return { nullptr, ERROR_TREE_LOOP };
        }
        rightChild = _recCopy(node->right);
    }
    if (rightChild.error)
    {
        leftChild.value->Delete();
        return { nullptr, rightChild.error };
    }

    TreeNodeResult copy = TreeNode::New(node->value, leftChild.value, rightChild.value);

    if (copy.error)
    {
        leftChild.value->Delete();
        rightChild.value->Delete();

        return { nullptr, copy.error };
    }

    node->id = oldId;

    return copy;
}

#ifdef SIZE_VERIFICATION
static ErrorCode _recUpdateParentNodeCount(TreeNode* node, ssize_t change)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return ERROR_TREE_LOOP;

    size_t oldId = node->id;
    node->id = BAD_ID;

    node->nodeCount += change;

    if (node->parent)
    {
        if (node->parent->left != node && node->parent->right != node)
            return ERROR_TREE_LOOP;
        RETURN_ERROR(_recUpdateParentNodeCount(node->parent, change));
    }

    node->id = oldId;

    return EVERYTHING_FINE;
}
#endif

ErrorCode Tree::Init(TreeNode* root)
{
    MyAssertSoft(root, ERROR_NULLPTR);

    this->root = root;
    #ifdef SIZE_VERIFICATION
    this->size = &root->nodeCount;
    #endif

    return EVERYTHING_FINE;
}

ErrorCode Tree::Init()
{
    TreeNodeResult rootRes = TreeNode::New(TREE_POISON, nullptr, nullptr);
    RETURN_ERROR(rootRes.error);

    this->root = rootRes.value;
    #ifdef SIZE_VERIFICATION
    this->size = &rootRes.value->nodeCount;
    #endif

    return EVERYTHING_FINE;
}

ErrorCode Tree::Destructor()
{
    ERR_DUMP_RET(this);

    RETURN_ERROR(this->root->Delete());

    this->root = nullptr;
    #ifdef SIZE_VERIFICATION
    this->size = nullptr;
    #endif
    
    return EVERYTHING_FINE;
}

ErrorCode Tree::Verify()
{
    if (!this->root)
        return ERROR_NO_ROOT;

    if (this->root->parent)
        return ERROR_TREE_LOOP;

    #ifdef SIZE_VERIFICATION
    if (*this->size > MAX_TREE_SIZE)
        return ERROR_BAD_SIZE;
    
    TreeNodeCountResult sizeRes = _recCountNodes(this->root);
    RETURN_ERROR(sizeRes.error);

    if (sizeRes.value != *this->size)
        return ERROR_BAD_TREE;
    #endif

    return EVERYTHING_FINE;
}

TreeNodeCountResult Tree::CountNodes()
{
    ERR_DUMP_RET_RESULT(this, SIZET_POISON);

    return _recCountNodes(this->root);
}

static TreeNodeCountResult _recCountNodes(TreeNode* node)
{
    MyAssertSoftResult(node, SIZET_POISON, ERROR_NULLPTR);
    if (node->id == BAD_ID)
        return { SIZET_POISON, ERROR_TREE_LOOP };

    size_t oldId = node->id;
    node->id = BAD_ID;

    size_t count = 1;

    TreeNodeCountResult countResult = {};

    if (node->left)
    {
        if (node->left->parent != node)
            return { SIZET_POISON, ERROR_TREE_LOOP };
        countResult = _recCountNodes(node->left);
        RETURN_ERROR_RESULT(countResult, SIZET_POISON);
    }

    count += countResult.value;

    if (node->right)
    {
        if (node->right->parent != node)
            return { SIZET_POISON, ERROR_TREE_LOOP };
        countResult = _recCountNodes(node->right);
        RETURN_ERROR_RESULT(countResult, SIZET_POISON);
    }

    count += countResult.value;

    node->id = oldId;

    return { count, EVERYTHING_FINE };
}

#ifdef SIZE_VERIFICATION
ErrorCode Tree::RecalculateNodes()
{
    ERR_DUMP_RET(this);

    return _recRecalcNodes(this->root);
}

ErrorCode _recRecalcNodes(TreeNode* node)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    if (node->id == BAD_ID)
        return ERROR_TREE_LOOP;

    size_t oldId = node->id;
    node->id = BAD_ID;

    node->nodeCount = 1;

    if (!node->left && !node->right)
        return EVERYTHING_FINE;
    
    if (node->left)
    {
        RETURN_ERROR(_recRecalcNodes(node->left));
        node->nodeCount += node->left->nodeCount;
    }
    if (node->right)
    {
        RETURN_ERROR(_recRecalcNodes(node->right));
        node->nodeCount += node->right->nodeCount;
    }

    node->id = BAD_ID;

    return EVERYTHING_FINE;
}
#endif

#define FONT_SIZE "10"
#define FONT_NAME "\"Fira Code Bold\""
#define BACK_GROUND_COLOR "\"#de97d4\""
#define TREE_COLOR "\"#ff7be9\""
#define NODE_COLOR "\"#fae1f6\""
#define NODE_FRAME_COLOR "\"#000000\""
#define ROOT_COLOR "\"#c95b90\""
#define FREE_HEAD_COLOR "\"#b9e793\""

ErrorCode Tree::Dump()
{
    static size_t DUMP_ITERATION = 0;

    MyAssertSoft(this->root, ERROR_NO_ROOT);

    if (HTML_FILE)
        fprintf(HTML_FILE, 
        "<h1>Iteration %zu</h1>\n"
        "<style>\n"
        ".content {\n"
        "max-width: 500px;\n"
        "margin: auto;\n"
        "}\n"
        "</style>,\n",
        DUMP_ITERATION);

    char outGraphPath[MAX_PATH_LENGTH] = "";
    sprintf(outGraphPath, "%s/Iteration%zu.dot", DOT_FOLDER, DUMP_ITERATION);

    FILE* outGraphFile = fopen(outGraphPath, "w");
    MyAssertSoft(outGraphFile, ERROR_BAD_FILE);

    fprintf(outGraphFile, 
    "digraph\n"
    "{\n"
    "rankdir = TB;\n"
    "node[shape = record, color = " NODE_FRAME_COLOR ", fontname = " FONT_NAME ", fontsize = " FONT_SIZE "];\n"
    "bgcolor = " BACK_GROUND_COLOR ";\n"
    );

    fprintf(outGraphFile, "TREE[rank = \"min\", style = \"filled\", fillcolor = " TREE_COLOR ", "
                          "label = \"{Tree|Error: %s|"
                          #ifdef SIZE_VERIFICATION
                          "Size: %zu|"
                          #endif
                          "<root>Root}\"];",
                          ERROR_CODE_NAMES[this->Verify()]
                          #ifdef SIZE_VERIFICATION
                          , *this->size
                          #endif
                          );

    fprintf(outGraphFile, "\nNODE_%p[style = \"filled\", fillcolor = " NODE_COLOR ", ",
                           this->root);
    if (this->root->value == TREE_POISON)
        fprintf(outGraphFile, "label = \"{Value:\\nPOISON|{<left>Left|<right>Right}}\"];\n");
    else
        fprintf(outGraphFile,
        "label = \"{Value:\\n" TREE_ELEMENT_SPECIFIER "|"
        "{<left>Left|<right>Right}}\"];\n", this->root->value);

    size_t MAX_DEPTH = MAX_TREE_SIZE;
    #ifdef SIZE_VERIFICATION
    MAX_DEPTH = min(*this->size, MAX_TREE_SIZE);
    #endif

    if (this->root->left)
        RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->left,  outGraphFile, 0, MAX_DEPTH));
    if (this->root->right)
        RETURN_ERROR(_recBuildCellTemplatesGraph(this->root->right, outGraphFile, 0, MAX_DEPTH));

    RETURN_ERROR(_recDrawGraph(this->root, outGraphFile, 0, MAX_DEPTH));
    fprintf(outGraphFile, "\n");
    fprintf(outGraphFile, "TREE:root->NODE_%p\n", this->root);

    fprintf(outGraphFile, "}\n");
    fclose(outGraphFile);

    char command[MAX_COMMAND_LENGTH] = "";
    sprintf(command, "dot %s -T png -o %s/Iteration%zu.png", outGraphPath, IMG_FOLDER, DUMP_ITERATION);
    system(command);

    if (HTML_FILE)
        fprintf(HTML_FILE, "<img src = \"%s/Iteration%zu.png\"/>\n", IMG_FOLDER, DUMP_ITERATION);

    DUMP_ITERATION++;

    return EVERYTHING_FINE;
}

static ErrorCode _recBuildCellTemplatesGraph(TreeNode* node, FILE* outGraphFile,
                                             size_t curDepth, const size_t maxDepth)
{
    MyAssertSoft(node, ERROR_NULLPTR);

    size_t nodeId = node->id;
    
    if (curDepth > maxDepth)
        return EVERYTHING_FINE;

    fprintf(outGraphFile, "NODE_%p[style = \"filled\", fillcolor = " NODE_COLOR ", ", node);
    fprintf(outGraphFile, "label = \"{Value:\\n");
    if (node->value == TREE_POISON)
        fprintf(outGraphFile, "POISON");
    else
        fprintf(outGraphFile, TREE_ELEMENT_SPECIFIER, node->value);
    fprintf(outGraphFile, "|id:\\n");

    if (node->id == BAD_ID)
        fprintf(outGraphFile, "BAD_ID");
    else
        fprintf(outGraphFile, "%zu", node->id);

    #ifdef SIZE_VERIFICATION
    fprintf(outGraphFile, "|node count:\\n%zu", node->nodeCount);
    #endif
    fprintf(outGraphFile, "|{<left>left|<right>right}}\"];\n");
    
    if (node->left)
        RETURN_ERROR(_recBuildCellTemplatesGraph(node->left,  outGraphFile, curDepth + 1, maxDepth));
    if (node->right)
        RETURN_ERROR(_recBuildCellTemplatesGraph(node->right, outGraphFile, curDepth + 1, maxDepth));

    return EVERYTHING_FINE;
}

#undef FONT_SIZE
#undef FONT_NAME
#undef BACK_GROUND_COLOR
#undef NODE_COLOR
#undef NODE_FRAME_COLOR
#undef ROOT_COLOR
#undef FREE_HEAD_COLOR

static ErrorCode _recDrawGraph(TreeNode* node, FILE* outGraphFile, size_t curDepth, const size_t maxDepth)
{
    MyAssertSoft(node, ERROR_NULLPTR);
    if (curDepth > maxDepth)
        return EVERYTHING_FINE;

    if (node->left)
    {
        fprintf(outGraphFile, "NODE_%p:left->NODE_%p;\n", node, node->left);
        RETURN_ERROR(_recDrawGraph(node->left, outGraphFile, curDepth + 1, maxDepth));
    }
    if (node->right)
    {
        fprintf(outGraphFile, "NODE_%p:right->NODE_%p;\n", node, node->right);
        RETURN_ERROR(_recDrawGraph(node->right, outGraphFile, curDepth + 1, maxDepth));
    }
    return EVERYTHING_FINE;
}

ErrorCode Tree::Print(const char* outPath)
{
    MyAssertSoft(outPath, ERROR_NULLPTR);
    ERR_DUMP_RET(this);

    FILE* outFile = fopen(outPath, "w");
    MyAssertSoft(outFile, ERROR_BAD_FILE);

    RETURN_ERROR(_recPrint(this->root, outFile));

    fclose(outFile);

    return EVERYTHING_FINE;
}

static ErrorCode _recPrint(TreeNode* node, FILE* outFile)
{
    if (!node)
    {
        fprintf(outFile, "nil%c", TREE_WORD_SEPARATOR);
        return EVERYTHING_FINE;
    }

    fprintf(outFile, "(%c" TREE_ELEMENT_SPECIFIER "%c", TREE_WORD_SEPARATOR, node->value, TREE_WORD_SEPARATOR);
    RETURN_ERROR(_recPrint(node->left, outFile));
    RETURN_ERROR(_recPrint(node->right, outFile));
    fprintf(outFile, ")%c", TREE_WORD_SEPARATOR);

    return EVERYTHING_FINE;
}

ErrorCode Tree::Read(const char* readPath)
{
    MyAssertSoft(readPath, ERROR_NULLPTR);

    Text input = CreateText(readPath, TREE_WORD_SEPARATOR);

    size_t wordNum = 0;

    TreeNodeResult rootRes = _recRead(&input, &wordNum);
    DestroyText(&input);

    RETURN_ERROR(rootRes.error);

    return this->Init(rootRes.value);
}

static TreeNodeResult _recRead(Text* input, size_t* wordNum)
{
    MyAssertSoftResult(*wordNum < input->numberOfWords, NULL, ERROR_INDEX_OUT_OF_BOUNDS);

    const String* string = &input->words[(*wordNum)++];
    ((char*)(string->text))[string->length] = '\0';

    const char* openBracket = strchr(string->text, '(');
    if (openBracket)
    {
        if (!StringIsEmptyChars(string->text, '('))
            return { nullptr, ERROR_SYNTAX };
        return _recReadOpenBracket(input, openBracket, wordNum);
    }

    const char* nil = strstr(string->text, "nil");
    if (nil)
        return { nullptr, EVERYTHING_FINE };
    return { nullptr, ERROR_SYNTAX };
}

static TreeNodeResult _recReadOpenBracket(Text* input, const char* openBracket, size_t* wordNum)
{
    if (!StringIsEmptyChars(openBracket + 1, '\0'))
        return { nullptr, ERROR_SYNTAX };

    const String* word = &input->words[(*wordNum)++];
    ((char*)(word->text))[word->length] = '\0';

    TreeElement_t value = TREE_POISON;
    int readChars = 0;

    if (sscanf(word->text, TREE_ELEMENT_SPECIFIER "%n", &value, &readChars) != 1 ||
        !StringIsEmptyChars(word->text + readChars, '\0'))
        return { nullptr, ERROR_SYNTAX };
    
    TreeNodeResult leftRes = _recRead(input, wordNum);
    RETURN_ERROR_RESULT(leftRes, nullptr);

    TreeNodeResult rightRes = _recRead(input, wordNum);
    RETURN_ERROR_RESULT(rightRes, nullptr);

    TreeNodeResult nodeRes = TreeNode::New(value, leftRes.value, rightRes.value);
    RETURN_ERROR_RESULT(nodeRes, nullptr);

    word = &input->words[(*wordNum)++];
    ((char*)(word->text))[word->length] = '\0';

    const char* closeBracket = strchr(word->text, ')');
    if (!closeBracket)
        return { nullptr, ERROR_SYNTAX };
    
    return nodeRes;
}


struct StringResult
{
    const char* value;
    ErrorCode error;
};

ErrorCode Tree::StartHtmlLogging()
{
    HTML_FILE = fopen(HTML_FILE_PATH, "w");
    MyAssertSoft(HTML_FILE, ERROR_BAD_FILE);

    fprintf(HTML_FILE, 
        "<style>\n"
        ".content {\n"
        "max-width: 500px;\n"
        "margin: auto;\n"
        "}\n"
        "</style>,\n"
        "<body>\n"
        "<div class=\"content\">");

    return EVERYTHING_FINE;
}

ErrorCode Tree::EndHtmlLogging()
{
    if (HTML_FILE)
    {
        fprintf(HTML_FILE, "</div>\n</body>\n");
        fclose(HTML_FILE);
    }
    HTML_FILE = NULL;

    return EVERYTHING_FINE;
}
