#pragma once

#include "Types.h"

void MergeSort(int* a, uint count)
{
    // 3, 4, 76, 9, 2
    // count = 5;
    // count / 2 = 2;
    // count - (count / 2) = 3;

    // *a == 3;
    // *(a + count / 2) == 76

    // Split array
    if (count <= 1)
        return;

    uint firstHalf = count / 2;
    uint secondHalf = count - firstHalf;
    MergeSort(a, firstHalf);
    MergeSort(a + firstHalf, secondHalf);

    // Merge 2 arrays
    int* x = a;
    int* y = a + firstHalf;
    for (int i = 0; i < firstHalf; ++i)
    {
        if (*x < *y)

    }
}
