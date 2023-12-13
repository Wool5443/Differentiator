#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "OneginFunctions.hpp"
#include "Utils.hpp"
#include "StringFunctions.hpp"
#include "Sort.hpp"

size_t _countWords(const char* string, char terminator);

const String* _split(const char* string, size_t numOfWords, char terminator);

int _stringCompareStartToEnd(const void* s1, const void* s2);

int _stringCompareEndToStart(const void* s1, const void* s2);

Text CreateText(const char* path, char terminator)
{
    MyAssertHard(path, ERROR_NULLPTR);

    Text text = {};

    text.size = GetFileSize(path);
    
    char* rawText = (char*)calloc(text.size + 2, sizeof(char));

    rawText[text.size] = terminator;
    rawText[text.size + 1] = '\0';

    FILE* file = fopen(path, "rb");
    MyAssertHard(file, ERROR_BAD_FILE, );
    MyAssertHard(text.size == fread(rawText, sizeof(char), text.size, file), ERROR_BAD_FILE, );
    fclose(file); 

    text.rawText = rawText;

    text.numberOfWords = _countWords(rawText, terminator);

    text.words = _split(text.rawText, text.numberOfWords, terminator);

    return text;
}

void DestroyText(Text* text)
{
    MyAssertHard(text, ERROR_NULLPTR, );
    free((void*)(text->words));
    free((void*)(text->rawText));
}

void SortTextWords(Text* text, StringCompareMethod sortType)
{
    switch (sortType)
    {
        case START_TO_END:
            Sort((void*)(text->words), text->numberOfWords, sizeof((text->words)[0]), _stringCompareStartToEnd);
            break;
        case END_TO_START:
            Sort((void*)(text->words), text->numberOfWords, sizeof((text->words)[0]), _stringCompareEndToStart);
            break;
        default:
            Sort((void*)(text->words), text->numberOfWords, sizeof((text->words)[0]), _stringCompareStartToEnd);
            break;
    }
}

void PrintRawText(const Text* text, FILE* file)
{
    fputs(text->rawText, file);
}

void PrintTextWords(const Text* text, FILE* file, char terminator)
{
    for (size_t i = 0; i < text->numberOfWords; i++)
    {
        const char* line = text->words[i].text;
        if (*line != terminator)
            StringPrint(file, line, terminator);
    }
}

int _stringCompareStartToEnd(const void* s1, const void* s2)
{
    return StringCompare((String*)s1, (String*)s2, START_TO_END, IGNORE_CASE, IGNORED_SYMBOLS);
}

int _stringCompareEndToStart(const void* s1, const void* s2)
{
    return StringCompare((String*)s1, (String*)s2, END_TO_START, IGNORE_CASE, IGNORED_SYMBOLS);
}

size_t _countWords(const char* string, char terminator)
{
    MyAssertHard(string, ERROR_NULLPTR, );

    size_t words = 1;
    const char* newWordSymbol = strchr(string, terminator);
    while (newWordSymbol != NULL)
    {
        words++;
        newWordSymbol = strchr(newWordSymbol + 1, terminator);
    }
    return words;
}

const String* _split(const char* string, size_t numOfWords, char terminator)
{
    MyAssertHard(string, ERROR_NULLPTR);

    String* textWords = (String*)calloc(numOfWords, sizeof(textWords[0]));

    MyAssertHard(textWords, ERROR_NO_MEMORY);

    const char* endCurWord = strchr(string, terminator);

    textWords[0] = {.text = string,
                     .length = (size_t)(endCurWord - string)};

    size_t i = 1;

    while (endCurWord)
    {
        textWords[i] = {};
        textWords[i].text = endCurWord + 1;
        endCurWord = strchr(endCurWord + 1, terminator);
        textWords[i].length = endCurWord ? (size_t)(endCurWord - textWords[i].text) : 0;
        i++;
    }

    return (const String*)textWords;
}
