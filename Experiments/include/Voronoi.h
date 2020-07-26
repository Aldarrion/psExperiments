
#include "Headers.h"
#include "stb/stb_image_write.h"

struct SeedPoint
{
    int16 X;
    int16 Y;
    uint Color;
};

#define POINT_DIST 5

float Sqr(float x)
{
    return x * x;
}

float Dist(int x1, int y1, int x2, int y2)
{
    return (float)sqrt(Sqr(x1 - x2) + Sqr(y1 - y2));
}

float DistSqr(int x1, int y1, int x2, int y2)
{
    return (float)(Sqr(x1 - x2) + Sqr(y1 - y2));
}

typedef void (*VoronoiFunc)(const SeedPoint*, uint, uint*, int, int);

void VoronoiNaive(const SeedPoint* seeds, uint seedCount, uint* img, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const SeedPoint* closestPoint = NULL;
            float closestDist = FLT_MAX;
            for (uint si = 0; si < seedCount; ++si)
            {
                float currDist = DistSqr(x, y, seeds[si].X, seeds[si].Y);
                if (currDist < closestDist)
                {
                    closestDist = currDist;
                    closestPoint = seeds + si;
                }
            }
            assert(closestPoint);
            if (closestDist <= POINT_DIST)
            {
                // Highlight the seed point
                *(img + y * width + x) = (0xffffffff - closestPoint->Color) | 0xff000000;
            }
            else 
            {
                *(img + y * width + x) = closestPoint->Color;
            }
        }
    }
}

#define OUTPUT_VORONOI_STEPS 1

void VoronoiJumpFloodFill(const SeedPoint* seeds, uint seedCount, uint* img, int width, int height)
{
    #if OUTPUT_VORONOI_STEPS
        const uint imgSize = width * height * sizeof(uint);
        uint* tempImg = (uint*)malloc(imgSize);
    #endif

    for (int i = 0; i < seedCount; ++i)
    {
        *(img + seeds[i].Y * width + seeds[i].X) = i + 1;
    }

    int16 step = (int16)Max(width, height);
    step /= 2;

    while (step > 0)
    {
        struct
        {
            int16 X;
            int16 Y;
        } neighbors[8]
        {
            { -step, -step },
            { -step, 0},
            { -step, step},

            { 0, -step },
            { 0, step},

            { step, -step },
            { step, 0},
            { step, step},
        };

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                uint* imBase = (img + y * width + x );
                if (*imBase == 0)
                    continue;

                for (int ni = 0; ni < 8; ++ni)
                {
                    const int jumpX = (x + neighbors[ni].X);
                    const int jumpY = (y + neighbors[ni].Y);
                    if (jumpX < 0 || jumpX >= width || jumpY < 0 || jumpY >= height)
                        continue;

                    uint* im = (img + jumpY * width + jumpX);
                    if (*im == 0)
                    {
                        *im = *imBase;
                    }
                    else
                    {
                        float currentDist   = DistSqr(x, y, seeds[*im - 1].X, seeds[*im - 1].Y);
                        float newDist       = DistSqr(x, y, seeds[*imBase - 1].X, seeds[*imBase - 1].Y);
                        if (newDist < currentDist)
                        {
                            *im = *imBase;
                        }
                    }
                }
            }
        }

        #if OUTPUT_VORONOI_STEPS
            memset(tempImg, 0, imgSize);

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const uint* im = (img + y * width + x );
                    uint color = 0;
                    if (*im != 0)
                        color = seeds[*im - 1].Color;
                    *(tempImg + y * width + x ) = color;
                }
            }

            char buff[128];
            sprintf(buff, "c:/tmp/voronoiStep_%05d.png", step);
            int writeOK = stbi_write_png(buff, width, height, 4, tempImg, sizeof(uint) * width);
            if (!writeOK)
                assert(!"Writing error, ensure that the directory exists");
        #endif

        step /= 2;
    }

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint* im = (img + y * width + x );
            assert(*im);

            float dist = DistSqr(x, y, seeds[*im - 1].X, seeds[*im - 1].Y);
            if (dist <= POINT_DIST)
            {
                // Highlight the seed point
                *(img + y * width + x) = (0xffffffff - seeds[*im - 1].Color) | 0xff000000;
            }
            else 
            {
                *(img + y * width + x) = seeds[*im - 1].Color;
            }
        }
    }

    #if OUTPUT_VORONOI_STEPS
        free(tempImg);
    #endif
}

void GenerateVoronoi(const char* file, const SeedPoint* seeds, uint seedCount, uint width, uint height, VoronoiFunc genFunc)
{
    const uint imgSize = width * height * sizeof(uint);
    uint* img = (uint*)malloc(imgSize);
    memset(img, 0, imgSize);

    genFunc(seeds, seedCount, img, width, height);

    int writeOK = stbi_write_png(file, width, height, 4, img, sizeof(uint) * width);
    if (!writeOK)
        assert(!"Writing error, ensure that the directory exists");

    free(img);
}

#undef POINT_DIST

