//! @file

#ifndef STRING_FUNTCIONS_HPP
#define STRING_FUNTCIONS_HPP

#include <stddef.h>
#include <stdio.h>

/** @struct String
 * @brief Represents a const string with its length.
 * 
 * @var String::text - data.
 * @var String::length - length of the string.
 * @var String::toFree - whether to free the string.
*/
struct String
{
    const char* text;
    size_t length;
    bool toFree;
};

/** @enum StringCompareMethod
 * 
 * @var StringCompareMethod::START_TO_END - compares strings starting from their beginnings.
 * @var StringCompareMethod::END_TO_START - compares strings starting from their endings.
*/
enum StringCompareMethod {START_TO_END, END_TO_START};

/** @enum CaseOptions
 * 
 * @var CaseOptions::IGNORE_CASE - ignore case when comparing.
 * @var CaseOptions::MIND_CASE - consider case when comparing.
*/
enum CaseOptions {IGNORE_CASE, MIND_CASE};

/**
 * @brief Counts the length of a null terminated string.
 *
 * @param [in] string - the string to find the length of.
 * 
 * @param [in] terminator - what the strings ends with.
 *
 * @return size_t length of the string.
*/
size_t StringLength(const char* string, char terminator);

/**
 * @brief Creates a String terminating with the terminator.
 * 
 * @param [in] text - the text for the String.
 * @param [in] terminator - what the string ends with.
 * @param [in] toFree - whether to free the string.
 * 
 * @return String.
*/
String CreateString(const char* text, char terminator, bool toFree);

/**
 * @brief Copies the source to the destination.
 * Safe because destination length must be given.
 *
 * @param [in, out] destination - where to copy.
 * @param [in] source - from where to copy.
 * @param [in] maxLength - how many elements destination can store.
 * @param [in] terminator - what the string ends with.
 *
 * @return char* to destination.
*/
char* StringCopy(char* destination, const char* source, size_t maxLength, char terminator);

/**
 * @brief Copies the entire source into destination.
 * Unsafe.
 *
 * @param [in, out] destination - where to copy.
 * @param [in] source - from where to copy.
 * @param [in] terminator - what the string ends with.
 * 
 * @return char* to destination.
*/
char* StringCopyAll(char* destination, const char* source, char terminator);

/**
 * @brief Concatenate destination and source.
 * Safe because length of destination is given.
 *
 * @param [in, out] destination - the string to append to.
 * @param [in] source - from where to append.
 * @param [in] maxLength - destination size.
 * @param [in] terminator - what the string ends with.
 *
 * @return char* to destination.
*/
char* StringCat(char* destination, const char* source, size_t maxLength, char terminator);

/**
 * @brief Compares 2 strings and return which is bigger.
 *
 * @param [in] s1, s2 - strings to compare.
 * @param [in] stringCompareMethod - enum which tells how to sort.
 * @param [in] caseOption - enum which tells whether to ignore case.
 * @param [in] filter - characters to ignore while comparing.
 * @param [in] terminator - what strings end with.
 *
 * @return >0 - s1 is bigger.
 * @return 0 - equal.
 * @return <0 - s2 is bigger.
*/
int StringCompare(String* s1, String* s2, 
                  StringCompareMethod stringCompareMethod,
                  CaseOptions caseOption, const char* filter);

/**
 * @brief Compares length elements of the strings and returns if they are equal.
 *
 * @param [in] s1, s2 - the strings to compare.
 * @param [in] length - the amount of characters to compare.
 * @param [in] terminator - what the strings end with.
 *
 * @return true - equal.
 * @return false - unequal.
*/
bool StringEqual(const char* s1, const char* s2, const size_t length, char terminator);

/**
 * @brief Finds the target substring in where and returns the pointer to it in where.
 *
 * Uses shift algorithm.
 *
 * @param [in] where - the string to find in.
 * @param [in] target - the string to find.
 * @param [in] terminator - what the string ends with.
 *
 * @return char* to the target substring in destination.
*/
char* StringFind(char* where, const char* target, char terminator);

/**
 * @brief Finds char target in where and return a pointer to its location.
 *
 * @param [in] where - the string to find the target in.
 * @param [in] target - the char to find.
 * @param [in] terminator - what the string ends with.
 *
 * @return char* to the first occurrence of target in where.
*/
char* StringFindChar(char* where, const char target, char terminator);

/**
 * @brief filters all filter chars in string.
 * 
 * @param [in, out] string - the string to filter.
 * @param [in] filter - the chats to filter out.
 * @param [in] terminator - what the string ends with.
 * 
 * @return char* to the string.
*/
char* StringFilter(char* string, const char* filter, char terminator);

/**
 * @brief Prints the string to file.
 * 
 * @param [in] file - the file to write to.
 * @param [in] string - what to print.
 * @param [in] terminator - what the string ends with.
*/
void StringPrint(FILE* file, const char* string, char terminator);

/**
 * @brief Checks if the input string consists entirely of empty space chars.
 * 
 * @param [in] string - the string to check.
 * 
 * @return 1 if true, 0 if false.
*/
int StringIsEmptyChars(const String* string);

/**
 * @brief Checks if the input char[] consists entirely of empty space chars.
 * 
 * @param [in] string - the string to check.
 * @param [in] terminator - what the string ends with.
 * 
 * @return 1 if true, 0 if false.
*/
int StringIsEmptyChars(const char* string, char terminator);

#endif
