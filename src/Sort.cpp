#include "Sort.hpp"
#include "Utils.hpp"
#include "time.h"

static void quickSort(void* start, void* end, size_t elementSize, CompareFunction_t* compareFunction);

static void* partition(void* left, void* right, size_t elementSize, CompareFunction_t compareFunction);

static void sort3Elements(void* data, size_t elementSize, CompareFunction_t compareFunction);

static void sort2Elements(void* data, size_t elementSize, CompareFunction_t compareFunction);

const void* MinArray(const void* data, size_t elementCount, size_t elementSize, CompareFunction_t* compareFunction)
{
	const void* min = data;
	for (size_t i = 1; i < elementCount; i++)
		if (compareFunction(min, data + i * elementSize) > 0)
		{
			min = data + i * elementSize;
		}
	return min;	
}

const void* MaxArray(const void* data, size_t elementCount, size_t elementSize, CompareFunction_t* compareFunction)
{
	const void* max = data;
	for (size_t i = elementSize; i < elementCount; i++)
		if (compareFunction(max, data + i * elementSize) < 0)
			max = data + i * elementSize;
	return max;	
}

void Sort(void* data, size_t elementCount, size_t elementSize, CompareFunction_t* compareFunction)
{
	MyAssertHard(data, ERROR_NULLPTR);
	MyAssertHard(compareFunction, ERROR_NO_COMPARATOR);

	quickSort(data, data + (elementCount - 1) * elementSize, elementSize, compareFunction);
	// selectionSort(data, elementCount, elementSize, compareFunction);
}

static void quickSort(void* start, void* end, size_t elementSize, CompareFunction_t* compareFunction)
{
	MyAssertHard(start, ERROR_NULLPTR);
	MyAssertHard(end, ERROR_NULLPTR);

	if (end <= start)
		return;
	
	size_t dataLength = (size_t)((char*)end - (char*)start) / elementSize + 1;

	switch (dataLength)
	{
	case 2:
		sort2Elements(start, elementSize, compareFunction);
		return;
	case 3:
		sort3Elements(start, elementSize, compareFunction);
		return;	
	default:
		break;
	}

	void* pivot = partition(start, end, elementSize, compareFunction);

	quickSort(start, pivot - elementSize, elementSize, compareFunction);
	quickSort(pivot + elementSize, end, elementSize, compareFunction);
}

static void* partition(void* start, void* end, size_t elementSize, CompareFunction_t compareFunction)
{
	MyAssertHard(start, ERROR_NULLPTR);
	MyAssertHard(end, ERROR_NULLPTR);
	
	srand((unsigned int)time(NULL));
	size_t arrayLength = ((size_t)end - (size_t)start) / elementSize + 1;

	// void* pivotPtr = start + arrayLength / 2 * elementSize;
	void* pivotPtr = start + ((size_t)rand() % (arrayLength - 2) + 1) * elementSize;

	Swap(start, pivotPtr, elementSize);
	void* pivotValue = start;
	start += elementSize;

	void* left  = start;
	void* right = end;

	int comp1 = 0, comp2 = 0;
	while (left < right)
	{
	    comp1 = compareFunction(left,  pivotValue);
		comp2 = compareFunction(right, pivotValue);
	
		while (comp1 < 0 && left < right)
		{
			left += elementSize;
			comp1 = compareFunction(left, pivotValue);
		}
	
		while (comp2 > 0 && left < right) 
		{
			right -= elementSize;
			comp2 = compareFunction(right, pivotValue);
		}

		if(comp1 == 0 && comp2 == 0)
			right -= elementSize;
		else
			Swap(left, right, elementSize);
	}

	start -= elementSize;

	if (comp1 >= 0)
		pivotPtr = left - elementSize;
	else
		pivotPtr = left;

	Swap(pivotValue, pivotPtr, elementSize);

	return pivotPtr;
}

static void sort3Elements(void* data, size_t elementSize, CompareFunction_t compareFunction)
{
	if (compareFunction(data, data + elementSize) < 0)
	{
		if (compareFunction(data, data + 2 * elementSize) > 0)
			Swap(data, data + 2 * elementSize, elementSize);
	}
	else
	{
		if (compareFunction(data + elementSize, data + 2 * elementSize) < 0)
		{
			Swap(data, data + 1 * elementSize, elementSize);
		}
		else
		{
			Swap(data, data + 2 * elementSize, elementSize);
		}
	}
	if (compareFunction(data + elementSize, data + 2 * elementSize) > 0)
		Swap(data + elementSize, data + 2 * elementSize, elementSize);
}

static void sort2Elements(void* data, size_t elementSize, CompareFunction_t compareFunction)
{
	if (compareFunction(data, data + elementSize) > 0)
		Swap(data, data + elementSize, elementSize);
}
