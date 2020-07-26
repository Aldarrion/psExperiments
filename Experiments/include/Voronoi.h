
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
                float currDist = Dist(x, y, seeds[si].X, seeds[si].Y);
                if (currDist < closestDist)
                {
                    closestDist = currDist;
                    closestPoint = seeds + si;
                }
            }
            assert(closestPoint);
            if (closestDist > POINT_DIST)
            {
                *(img + y * width + x) = closestPoint->Color;
            }
            else
            {
                *(img + y * width + x) = 0xffffffff;
            }
        }
    }
}

void GenerateVoronoi(const SeedPoint* seeds, uint seedCount, uint width, uint height)
{
    const uint imgSize = width * height * sizeof(uint);
    uint* img = (uint*)malloc(imgSize);
    memset(img, 0, imgSize);

    VoronoiNaive(seeds, seedCount, img, width, height);
    const char* naiveFile = "c:/tmp/voronoiNaive.png";

    //int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
    int writeOK = stbi_write_png(naiveFile, width, height, 4, img, sizeof(uint) * width);
    if (!writeOK)
        assert(!"Writing error, ensure that the directory exists");

    free(img);
}

#undef POINT_DIST

