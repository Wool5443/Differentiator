#include <ctype.h>
#include "StringFunctions.hpp"
#include "Utils.hpp"
#include "MinMax.hpp"

const int INCLUDE_NULL_TERMINATOR_FIX = 1, FOUND_STRING = -1, ALPHABET_LENGTH = 256;

int findShift(const char* where, const char* target, const size_t place, const size_t targetLength);

int stringCompareStartToEnd(String* s1, String* s2, CaseOptions caseOption, const char* filter);

int stringCompareEndToStart(String* s1, String* s2, CaseOptions caseOption, const char* filter);

int checkFilter(char c, const char* filter);

size_t StringLength(const char* string, char terminator)
{
    MyAssertHard(string, ERROR_NULLPTR);

    size_t length = 0;
    while (*string++ != terminator)
        length++;

    return length;
}

String CreateString(const char* text, char terminator, bool toFree)
{
    return { .text = text, .length = StringLength(text, terminator), toFree };
}

char* StringCopy(char* destination, const char* source, size_t maxLength, char terminator)
{
    MyAssertHard(destination, ERROR_NULLPTR, );
    MyAssertHard(source, ERROR_NULLPTR, );

    size_t sourceLength = StringLength(source, terminator);
    size_t numOfElToCopy = min(sourceLength, maxLength - INCLUDE_NULL_TERMINATOR_FIX);

    MyAssertHard(destination + maxLength <= source
        || source + sourceLength < destination,
        ERROR_OVERLAP,
        );

    for (size_t i = 0; i < numOfElToCopy; i++)
        destination[i] = source[i];

    destination[numOfElToCopy] = terminator;

    return destination;
}

char* StringCopyAll(char* destination, const char* source, char terminator)
{
    MyAssertHard(destination, ERROR_NULLPTR);
    MyAssertHard(source, ERROR_NULLPTR);

    size_t sourceLength = StringLength(source, terminator);

    MyAssertHard(destination + sourceLength < source
        || source + sourceLength < destination,
        ERROR_OVERLAP,
        );

    for (size_t i = 0; i < sourceLength - 1; i++)
        destination[i] = source[i];

    destination[sourceLength - 1] = terminator;

    return destination;
}

char* StringCat(char* destination, const char* source, size_t maxLength, char terminator)
{
    MyAssertHard(destination, ERROR_NULLPTR);
    MyAssertHard(source, ERROR_NULLPTR);

    size_t destinationLength = StringLength(destination, terminator);

    return StringCopy(destination + destinationLength, source, maxLength - destinationLength, terminator) - destinationLength;
}

int StringCompare(String* s1, String* s2, StringCompareMethod stringCompareMethod, 
                  CaseOptions caseOption, const char* filter)
{
    MyAssertHard(s1, ERROR_NULLPTR);
    MyAssertHard(s2, ERROR_NULLPTR);

    switch (stringCompareMethod)
    {
        case START_TO_END:
            return stringCompareStartToEnd(s1, s2, caseOption, filter);
        case END_TO_START:
            return stringCompareEndToStart(s1, s2, caseOption, filter);
        default:
            return 0;
    }
}

int stringCompareStartToEnd(String* s1, String* s2, CaseOptions caseOption, const char* filter)
{
    MyAssertHard(s1, ERROR_NULLPTR);
    MyAssertHard(s2, ERROR_NULLPTR);

    const char* s1Text = s1->text;
    const char* s2Text = s2->text;

    if (s1Text == s2Text)
        return 0;

    const char* const s1End = s1Text + s1->length;
    const char* const s2End = s2Text + s2->length;

    while (s1Text < s1End && s2Text < s2End)
    {
        if (*s1Text == *s2Text || (caseOption == IGNORE_CASE && tolower(*s1Text) == tolower(*s2Text)))
        {
            s1Text++;
            s2Text++;
            continue;
        }
        int checkS1 = checkFilter(*s1Text, filter);
        int checkS2 = checkFilter(*s2Text, filter);

        if (checkS1 && checkS2)
            break;
            
        s1Text += !checkS1;
        s2Text += !checkS2;
    }

    return caseOption == IGNORE_CASE ? tolower(*s1Text) - tolower(*s2Text) : *s1Text - *s2Text;
}

int stringCompareEndToStart(String* s1, String* s2, CaseOptions caseOption, const char* filter)
{
    MyAssertHard(s1, ERROR_NULLPTR);
    MyAssertHard(s2, ERROR_NULLPTR);

    const char* s1Text = s1->text;
    const char* s2Text = s2->text;

    if (s1Text == s2Text)
        return 0;

    const char* s1Arrow = s1Text + s1->length - 1;
    const char* s2Arrow = s2Text + s2->length - 1;

    while (s1Arrow >= s1Text && s2Arrow >= s2Text)
    {
        if (*s1Arrow == *s2Arrow || (caseOption == IGNORE_CASE && tolower(*s1Arrow) == tolower(*s2Arrow)))
        {
            s1Arrow--;
            s2Arrow--;
            continue;
        }
        
        int checkS1Arrow = checkFilter(*s1Arrow, filter);
        int checkS2Arrow = checkFilter(*s2Arrow, filter);

        if (checkS1Arrow && checkS2Arrow)
            break;

        s1Arrow -= !checkS1Arrow;
        s2Arrow -= !checkS2Arrow;
    }

    return caseOption == IGNORE_CASE ? tolower(*s1Arrow) - tolower(*s2Arrow) : *s1Arrow - *s2Arrow;
}

int checkFilter(char c, const char* filter)
{
    MyAssertHard(filter, ERROR_NULLPTR);

    while (*filter != 0)
    {
        if (c == *filter)
            return 0; // to be filtered out
        filter++;
    }
    return 1;         // to stay
}

bool StringEqual(const char* s1, const char* s2, const size_t length, char terminator)
{
    MyAssertHard(s2, ERROR_NULLPTR);
    MyAssertHard(s1, ERROR_NULLPTR);

    size_t s1Length = StringLength(s1, terminator);
    size_t s2Length = StringLength(s2, terminator);

    if (s1Length < length || s2Length < length)
        return false;

    for (size_t i = 0; i < length; i++)
        if (s1[i] != s2[i])
            return false;

    return true;
}

char* StringFind(char* where, const char* target, char terminator)
{
    MyAssertHard(where, ERROR_NULLPTR);
    MyAssertHard(target, ERROR_NULLPTR);

    size_t whereLength = StringLength(where, terminator);
    size_t targetLength = StringLength(target, terminator);
    if (whereLength < targetLength)
        return NULL;

    size_t shifts[ALPHABET_LENGTH] = {};

    for (size_t i = 0; i < ALPHABET_LENGTH; i++)
        shifts[i] = targetLength;

    for (size_t i = 2; i < targetLength + 1; i++)
    {
        int c = target[targetLength - i];
        shifts[c] = min(shifts[c], i - 1);
    }

    size_t place = 0;

    while (place + targetLength - 1 < whereLength)
    {
        int shiftChar = findShift(where, target, place, targetLength);
        if (shiftChar == FOUND_STRING)
            return where + place;
        place += shifts[shiftChar];
    }

    return NULL;
}

int findShift(const char* where, const char* target, const size_t place, const size_t targetLength)
{
    MyAssertHard(where, ERROR_NULLPTR);
    MyAssertHard(target, ERROR_NULLPTR);

    for (size_t i = 0; i < targetLength; i++)
        if (where[place + targetLength - 1 - i] != target[targetLength - 1 - i])
            return where[place + targetLength - 1 - i];
    return FOUND_STRING;
}

char* StringFindChar(char* where, const char target, char terminator)
{
    MyAssertHard(where, ERROR_NULLPTR);

    while (*where != terminator)
    {
        if (*where == target)
            return where;
        where++;
    }

    return NULL;
}

char* StringFilter(char* string, const char* filter, char terminator)
{
    MyAssertHard(string, ERROR_NULLPTR);
    MyAssertHard(filter, ERROR_NULLPTR);

    const char* readPtr = string;
    char* writePtr = string;

    while (*readPtr != terminator)
    {
        char c = *readPtr++;

        if (checkFilter(c, filter))
            *writePtr++ = c;
    }
    *writePtr = '\0';

    return string;
}

void StringPrint(FILE* file, const char* string, char terminator)
{
    MyAssertHard(string, ERROR_NULLPTR);
    MyAssertHard(file, ERROR_BAD_FILE);

    while (*string != terminator && *string != 0)
        putc(*string++, file);
    putc('\n', file);
}

int StringIsEmptyChars(const String* string)
{
    MyAssertHard(string, ERROR_NULLPTR);

    for (size_t i = 0; i < string->length; i++)
        if (!isspace(string->text[i]))
            return 0;
    return 1;
}

int StringIsEmptyChars(const char* string, char terminator)
{
    MyAssertHard(string, ERROR_NULLPTR);

    while (isspace(*string) && *string != terminator)
        string++;

    if (*string == terminator)
        return 1;
    return 0;
}
