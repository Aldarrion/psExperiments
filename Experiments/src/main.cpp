#include "SparseArray.h"
#include "Array.h"

#include "Heap.h"
#include "SortedArray.h"

#include <chrono>
#include <cstdio>

void SparseArrayTest()
{
    SparseArray<const char*> a;

    a.Insert(0, "abc");
    a.Insert(23, "xxx");
    a.Insert(123, "123");
    a.Insert(355, "355");

    printf("---- Strings ----\n");
    printf("Element at %d is %s\n", 0, a[0]);
    printf("Element at %d is %s\n", 23, a[23]);
    printf("Element at %d is %s\n", 123, a[123]);
    printf("Element at %d is %s\n", 355, a[355]);

    a.Remove(123);
    a[23] = "new value";

    printf("---- Strings 2 ----\n");
    printf("Element at %d is %s\n", 0, a[0]);
    printf("Element at %d is %s\n", 23, a[23]);
    printf("Element at %d is %s\n", 355, a[355]);

    struct Point
    {
        int x_;
        int y_;

        Point(int x, int y) : x_(x), y_(y) {}
    };

    SparseArray<Point> points;

    points.Insert(0, Point(1, 1));
    points.Insert(223, Point(22, 3));
    points.Insert(22, Point(2, 2));
    points.Insert(89, Point(8, 9));

    printf("---- Points ----\n");
    printf("Element at %d is %d %d\n", 0, points[0].x_, points[0].y_);
    printf("Element at %d is %d %d\n", 223, points[223].x_, points[223].y_);
    printf("Element at %d is %d %d\n", 22, points[22].x_, points[22].y_);
    printf("Element at %d is %d %d\n", 89, points[89].x_, points[89].y_);

    points.Remove(89);
    points[22].x_ = 999999;

    printf("---- Points 2 ----\n");
    printf("Element at %d is %d %d\n", 0, points[0].x_, points[0].y_);
    printf("Element at %d is %d %d\n", 223, points[223].x_, points[223].y_);
    printf("Element at %d is %d %d\n", 22, points[22].x_, points[22].y_);

    auto pc = PopCount64((uint64)-1);
    printf("PopCount -1: %ld", pc);
}

void ArrayTest()
{
    Array<int> a;
    int size = 33;
    for (int i = 0; i < size; ++i)
    {
        a.Add(i);
    }

    printf("--- Array after add\n");
    for (int i = 0; i < a.Count(); ++i)
    {
        printf("\tArr[%d] == %d\n", i, a[i]);
    }

    for (int i = (size - 1) / 2; i >= 0; --i)
    {
        a.Remove(i * 2);
    }

    printf("--- Array after remove\n");
    for (int i = 0; i < a.Count(); ++i)
    {
        printf("\tArr[%d] == %d\n", i, a[i]);
    }

    a.Insert(a.Count(), 999);
    a.Insert(0, -123);
    a.Insert(3, 333);

    printf("--- Array after insert\n");
    for (int i = 0; i < a.Count(); ++i)
    {
        printf("\tArr[%d] == %d\n", i, a[i]);
    }
}

void HeapVsSortedArrayBench()
{
    constexpr int ITER = 1'00'000;

    {
        int checksum = 0;
        SortedArray sa;
        srand(42);

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER; ++i)
        {
            bool remove = (rand() % 3) == 0;
            if (sa.Count() && remove)
            {
                checksum += sa.RemoveMin();
            }
            else
            {
                sa.Add(rand());
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = (end - start).count() / (1000.0f * 1000 * 1000);

        printf("SortedArray chsm: %d, elapsed: %f seconds\n", checksum, elapsed);
    }

    {
        int checksum = 0;
        Heap sa;
        srand(42);

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITER; ++i)
        {
            bool remove = (rand() % 3) == 0;
            if (sa.Count() && remove)
            {
                checksum += sa.RemoveMin();
            }
            else
            {
                sa.Add(rand());
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = (end - start).count() / (1000.0f * 1000 * 1000);

        printf("SortedArray chsm: %d, elapsed: %f seconds\n", checksum, elapsed);
    }
}

int main()
{
    //SparseArrayTest();
    //ArrayTest();

    HeapVsSortedArrayBench();

    return 0;
}
