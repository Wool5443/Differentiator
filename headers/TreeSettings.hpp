#ifndef TREE_SETTINGS_INI
#define TREE_SETTINGS_INI

#include <math.h>

enum TreeElementType
{
    OPERATION_TYPE,
    NUMBER_TYPE,
    VARIABLE_TYPE,
};
enum Operation
{
#define DEF_FUNC(name, ...) \
name,

#include "DiffFunctions.hpp"

#undef DEF_FUNC
};

#define DEF_FUNC(name, prior, ...) \
[[maybe_unused]] static int name ## _PRIORITY = prior;

#include "DiffFunctions.hpp"

#undef DEF_FUNC

struct TreeElement
{
    TreeElementType type;
    int priority;
    union val
    {
        Operation operation;
        char var;
        double number;
    } value;
};

typedef TreeElement TreeElement_t;

[[maybe_unused]] static const TreeElement_t TREE_POISON = {};
[[maybe_unused]] static const size_t MAX_TREE_SIZE = 1000;
[[maybe_unused]] static const char TREE_WORD_SEPARATOR = ' ';

#define TEX_WRITE
#define SIZE_VERIFICATION

[[maybe_unused]] static const char* DOT_FOLDER = "log/dot";
[[maybe_unused]] static const char* IMG_FOLDER = "log/img";
[[maybe_unused]] static const char* HTML_FILE_PATH = "log.html";

static const size_t BAD_ID = 0;

#endif
