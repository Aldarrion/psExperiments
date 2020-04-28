#include <cstdio>

#include "SparseArray.h"

int main()
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

    return 0;
}
