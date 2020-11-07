#include "SparseArray.h"
#include "Array.h"

#include "Heap.h"
#include "SortedArray.h"

#include "Voronoi.h"

#include "Ecs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <chrono>
#include <cstdio>
#include <vector>

using namespace hs;

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

void VoronoiTest()
{
    constexpr uint seedCount = 5;
    SeedPoint seeds[seedCount];
    seeds[0] = SeedPoint{ 100, 100, 0xffff0000 };
    seeds[1] = SeedPoint{ 800, 800, 0xff00ff00 };
    seeds[2] = SeedPoint{ 456, 120, 0xff0000ff };
    seeds[3] = SeedPoint{ 500, 500, 0xff00ffff };
    seeds[4] = SeedPoint{ 100, 700, 0xffff00ff };

    const char* naiveFile = "c:/tmp/voronoiNaive.png";
    const char* jffFile = "c:/tmp/voronoiJFF.png";

    GenerateVoronoi(naiveFile, seeds, seedCount, 1024, 1024, &VoronoiNaive);
    GenerateVoronoi(jffFile, seeds, seedCount, 1024, 1024, &VoronoiJumpFloodFill);
}

struct Position
{
    int x;
    int y;
};

struct Velocity
{
    float x;
    float y;
};

void EcsTest()
{
    using namespace archetypeECS;

    TypeInfo<Entity_t>::InitTypeId();

    EcsWorld world;

    constexpr uint ENT_COUNT = 8;
    Array<Entity_t> entities;

    for (int i = 0; i < ENT_COUNT; ++i)
    {
        entities.Add(world.CreteEntity());
    }

    world.DeleteEntity(entities[5]);
    entities.Remove(5);

    world.DeleteEntity(entities[0]);
    entities.Remove(0);

    entities.Add(world.CreteEntity());
    entities.Add(world.CreteEntity());

    world.DeleteEntity(entities[3]);
    entities.Remove(3);

    uint *ent, entCount;
    world.GetEntities(ent, entCount);

    for (int i = 0; i < entCount; ++i)
    {
        printf("%d\n", ent[i]);
    }

    TypeInfo<Position>::InitTypeId();
    TypeInfo<Velocity>::InitTypeId();

    printf("\nEntities:\n");

    EcsWorld::Iter<Entity_t> it(&world);
    it.Each([](Entity_t eid)
    {
        printf("%d\n", eid);
    });

    EcsWorld::Iter<Position, const Velocity> posVelIt(&world);
    auto editPosVel = [](Position& pos, const Velocity& vel)
    {
        printf("Pos: %d, %d, Vel: %f, %f\n", pos.x, pos.y, vel.x, vel.y);
        pos.x += 10;
    };

    printf("\nPos Vel:\n");
    posVelIt.Each(editPosVel);

    for (int i = 0; i < 4; ++i)
    {
        world.SetComponent(entities[i], Position{ i, i + 1 });
        world.SetComponent(entities[i], Velocity{ 1.0f / (i + 1), 1 + 1.0f / (i + 1) });
    }

    printf("\nPos Vel:\n");
    posVelIt.Each(editPosVel);

    printf("\nVel Pos:\n");
    EcsWorld::Iter<const Velocity, const Position> velPosIt(&world);
    auto printVelPos = [](const Velocity& vel, const Position& pos)
    {
        printf("Pos: %d, %d, Vel: %f, %f\n", pos.x, pos.y, vel.x, vel.y);
    };

    velPosIt.Each(printVelPos);

    int x = 0;
}


// Include here because of clases with flecs macros
#include "flecs/flecs.h"

void FlecsTest()
{
    if (0)
    {
        flecs::world world;
        // Create a new empty entity. Entity names are optional.
        constexpr uint ENT_COUNT = 16;
        flecs::entity entities[ENT_COUNT];
        for (int i = 0; i < ENT_COUNT; ++i)
        {
            entities[i] = world.entity();
            entities[i].set<Position>({i, i});
        }

        world.system<const Position>()
            .each([](flecs::entity e, const Position& p)
        {
            printf("  %d\n", p.x);
        });

        printf("First\n");
        world.progress();

        entities[5].set<Velocity>({5.0f, 5.0f});
        entities[10].set<Velocity>({10.0f, 10.0f});

        printf("Second\n");
        world.progress();

        entities[10].remove<Velocity>();

        printf("Third\n");
        world.progress();

        int x = 0;
    }
}

int main()
{
    //SparseArrayTest();
    //ArrayTest();

    //HeapVsSortedArrayBench();

    //VoronoiTest();

    EcsTest();

    FlecsTest();

    return 0;
}
