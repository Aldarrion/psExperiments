#include <cstdio>

#include "SparseArray.h"

int main()
{
    SparseArray a;

    a.Insert(0, "abc");
    a.Insert(23, "xxx");
    a.Insert(123, "123");
    a.Insert(355, "355");

    printf("Element at %d is %s\n", 0, a[0]);
    printf("Element at %d is %s\n", 23, a[23]);
    printf("Element at %d is %s\n", 123, a[123]);
    printf("Element at %d is %s\n", 355, a[355]);

    auto pc = PopCount64((uint64)-1);
    printf("PopCount -1: %ld", pc);

    return 0;
}
