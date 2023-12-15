#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include "Utils.hpp"

const double ABSOLUTE_TOLERANCE = 1e-5;

bool IsEqual(const double x1, const double x2)
{
	return fabs(x1 - x2) < ABSOLUTE_TOLERANCE;
}

void Swap(void* a, void* b, size_t size)
{
    char* _a = (char*)a;
    char* _b = (char*)b;

    for (size_t curByte = 0; curByte < size; curByte++)
    {
        char _temp = _a[curByte];
        _a[curByte] = _b[curByte];
        _b[curByte] = _temp;
    }
}

void ClearBuffer(FILE* where)
{
	int c = fgetc(where);
	while (c != '\n' && c != EOF) { c = fgetc(where); }
}

bool CheckInput(FILE* where)
{
	int c = fgetc(where);
	while (c == ' ' || c == '\t') { c = fgetc(where); }

	return c == '\n';
}

char* ReadFileToBuf(const char* filePath)
{
	if (!filePath)
		return nullptr;

	size_t fileSize = GetFileSize(filePath);

	char* buf = (char*)calloc(fileSize + 2, 1);
	if (!buf)
		return nullptr;

	FILE* file = fopen(filePath, "r");
	if (!file)
		return nullptr;

	if (fread(buf, 1, fileSize, file) != fileSize)
		return nullptr;

	return buf;
}

void SetConsoleColor(FILE* where, enum Color color)
{
	fprintf(where, "\033[0;%dm", (int)color);
}

size_t GetFileSize(const char* path)
{
    MyAssertHard(path, ERROR_NULLPTR, );

    struct stat result = {};
    stat(path, &result);

    return (size_t)result.st_size;
}

#define mmix(h, k) do { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; } while (0)

unsigned int CalculateHash(const void *key, size_t len, unsigned int seed)
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	unsigned int l = (unsigned int)len;

	const unsigned char* data = (const unsigned char *)key;

	unsigned int h = seed;
	unsigned int k;

	while(len >= 4)
	{
		k = *(unsigned int*)data;

		mmix(h,k);

		data += 4;
		len -= 4;
	}

	unsigned int t = 0;

	switch(len)
	{
	case 3:
		t ^= (unsigned int)(data[2] << 16);
		break;
	case 2:
		t ^= (unsigned int)(data[1] << 8);
		break;
	case 1:
		t ^= data[0];
		break;
	default: 
		break;
	};

	mmix(h,t);
	mmix(h,l);

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}
