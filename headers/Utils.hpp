//! @file

#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define TESTINGs

/** @enum Color
 * @brief Represents colors for @see SetConsoleColor
 */
enum Color
{
    COLOR_BLACK = 30,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
};

/** @enum ErrorCode
 * @brief Represents possible error codes for @see MyAssertHard()
 */
enum ErrorCode 
{
    EVERYTHING_FINE = 0, ERROR_NULLPTR, ERROR_BAD_NUMBER, ERROR_BAD_FILE, ERROR_OVERLAP,
    ERROR_INDEX_OUT_OF_BOUNDS, ERROR_NO_MEMORY, ERROR_NO_COMPARATOR, ERROR_BAD_SIZE,
    ERROR_BAD_VALUE, ERROR_DEAD_CANARY, ERROR_BAD_HASH, ERROR_ZERO_DIVISION,
    ERROR_SYNTAX, ERROR_WRONG_LABEL_SIZE, ERROR_TOO_MANY_LABELS,
    ERROR_NOT_FOUND, ERROR_BAD_FIELDS, ERROR_BAD_TREE, ERROR_NO_ROOT,
    ERROR_TREE_LOOP, EXIT,
};

static const char* ERROR_CODE_NAMES[] =
{
    "EVERYTHING_FINE", "ERROR_NULLPTR", "ERROR_BAD_NUMBER", "ERROR_BAD_FILE", "ERROR_OVERLAP",
    "ERROR_INDEX_OUT_OF_BOUNDS", "ERROR_NO_MEMORY", "ERROR_NO_COMPARATOR", "ERROR_BAD_SIZE",
    "ERROR_BAD_VALUE", "ERROR_DEAD_CANARY", "ERROR_BAD_HASH", "ERROR_ZERO_DIVISION",
    "ERROR_SYNTAX", "ERROR_WRONG_LABEL_SIZE", "ERROR_TOO_MANY_LABELS",
    "ERROR_NOT_FOUND", "ERROR_BAD_FIELDS", "ERROR_BAD_TREE", "ERROR_NO_ROOT",
    "ERROR_TREE_LOOP", "EXIT",
};

static const size_t SIZET_POISON = (size_t)-1;

#define RETURN_ERROR(error, ...)                                                                                            \
do                                                                                                                          \
{                                                                                                                           \
    __typeof__(error) _error = error;                                                                                       \
    if (_error)                                                                                                             \
    {                                                                                                                       \
        __VA_ARGS__;                                                                                                        \
        return _error;                                                                                                      \
    }                                                                                                                       \
} while (0)

#define RETURN_ERROR_RESULT(result, poison, ...)                                                                            \
do                                                                                                                          \
{                                                                                                                           \
    __typeof__(result) _result = result;                                                                                      \
    if (_result.error)                                                                                                      \
    {                                                                                                                       \
        __VA_ARGS__;                                                                                                        \
        return { poison, _result.error };                                                                                   \
    }                                                                                                                       \
} while (0)

/**
 * @brief Hard assert which tells the file, function and line where the error occurred.
 *
 * @param [in] statement - the condition to check.
 * @param [in] error - what can happen @see ErrorCode.
 * @param [in] exitCode - code to perform before exiting the program.
 *
 * @note If there is nothing to perform pass nothing.
 */
#define MyAssertHard(statement, error, ...)                                                                                 \
if (!(statement))                                                                                                           \
do {                                                                                                                        \
    SetConsoleColor(stderr, COLOR_RED);                                                                                     \
    fprintf(stderr, "%s in %s in %s in line: %d\n", ERROR_CODE_NAMES[error], __FILE__, __PRETTY_FUNCTION__, __LINE__);      \
    SetConsoleColor(stderr, COLOR_WHITE);                                                                                   \
    __VA_ARGS__;                                                                                                            \
    exit(error);                                                                                                            \
} while(0)

/**
 * @brief Transforms a given name into a string.
 * 
 * @param [in] VALUE - the thing to transform.
*/
#define ValueToString(value) #value

/**
 * @brief Soft assert which tells the file, function and line where the error occurred.
 *
 * @param [in] statement - the condition to check.
 * @param [in] error - what can happen @see ErrorCode.
 * @param [in] exitCode - code to perform before exiting the program.
 *
 * @note If there is nothing to perform pass nothing.
 * 
 * @return ErrorCode
 */
#define MyAssertSoft(statement, error, ...)                                                                                 \
if (!(statement))                                                                                                           \
do {                                                                                                                        \
    SetConsoleColor(stderr, COLOR_RED);                                                                                     \
    fprintf(stderr, "%s in %s in %s in line: %d\n", ERROR_CODE_NAMES[error], __FILE__, __PRETTY_FUNCTION__, __LINE__);      \
    SetConsoleColor(stderr, COLOR_WHITE);                                                                                   \
    __VA_ARGS__;                                                                                                            \
    return error;                                                                                                           \
} while(0)

/**
 * @brief Soft assert which tells the file, function and line where the error occurred.
 *
 * @param [in] statement - the condition to check.
 * @param [in] value - the value to form result struct.
 * @param [in] error - what can happen @see ErrorCode.
 * @param [in] exitCode - code to perform before exiting the program.
 *
 * @note If there is nothing to perform pass nothing.
 * 
 * @return Result Struct.
 */
#define MyAssertSoftResult(statement, value, error, ...)                                                                    \
if (!(statement))                                                                                                           \
do {                                                                                                                        \
    SetConsoleColor(stderr, COLOR_RED);                                                                                     \
    fprintf(stderr, "%s in %s in %s in line: %d\n", ERROR_CODE_NAMES[error], __FILE__, __PRETTY_FUNCTION__, __LINE__);      \
    SetConsoleColor(stderr, COLOR_WHITE);                                                                                   \
    __VA_ARGS__;                                                                                                            \
    return { value, error };                                                                                                  \
} while(0)

/**
 * @brief Struct to contain where some variable was created.
 * 
 * @var Owner::fileName - the file where the variable was created.
 * @var Owner::line - the line where it was created.
 * @var Owner::name - the name of the variable.
*/
struct SourceCodePosition
{
    const char* fileName;
    size_t line;
    const char* name;
};

/**
 * @brief Tells if 2 doubles are equal.
 *
 * @param x1
 * @param x2
 *
 * @return true The numbers are equal.
 * @return false The numbers are not equal.
 */
bool IsEqual(const double x1, const double x2);

/**
 * @brief swaps 2 elements a and b in memory.
 * 
 * @param [in] a, b - elements to swap.
 * @param [in] size - size of the elements.
*/
void Swap(void* a, void* b, size_t size);

/**
 * @brief Clears stdin.
 * 
 * @param [in] where - file stream which buffer to clean.
 */
void ClearBuffer(FILE* where);

/**
 * @brief Check if the user input contains anything but scanned data.
 *
 * @param [in] where - file stream where to input.
 * 
 * @return true Everything is clear.
 * @return false User entered something odd.
 */
bool CheckInput(FILE* where);

/**
 * @brief Reads a file to a buffer and returns it.
 * 
 * @param [in] filePath path to the file.
 * @return char* buffer.
 */
char* ReadFileToBuf(const char* filePath);

/**
 * @brief Set the color of either stderr or stdout
 *
 * @param where - stderr or stdout
 * @param color - @see Color
 */
void SetConsoleColor(FILE* where, enum Color color);

/**
 * @brief Get the file size.
 * 
 * @param path to the file.
 * @return size.
 */
size_t GetFileSize(const char* path);

unsigned int CalculateHash(const void *key, size_t len, unsigned int seed);

#endif