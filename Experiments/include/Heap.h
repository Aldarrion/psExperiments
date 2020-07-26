#pragma once

#include "Array.h"

#include <cmath>
#include <cassert>

//------------------------------------------------------------------------------
void Swap(int& a, int& b)
{
    int tmp = a;
    a = b;
    b = tmp;
}

//------------------------------------------------------------------------------
class Heap
{
public:
    //------------------------------------------------------------------------------
    size_t Count()
    {
        return values_.Count();
    }

    //------------------------------------------------------------------------------
    void Add(int x)
    {
        values_.Add(x);

        size_t idx = values_.Count() - 1;

        while (idx != 0)
        {
            size_t parent = 0;
            if (idx & 1)
                parent = (idx - 1) / 2;
            else
                parent = (idx - 2) / 2;

            if (values_[idx] < values_[parent])
            {
                Swap(values_[idx], values_[parent]);
                idx = parent;
            }
            else
            {
                break;
            }
        }
    }

    //------------------------------------------------------------------------------
    int RemoveMin()
    {
        assert(values_.Count());

        int ret = values_[0];
        values_[0] = values_.Last();
        values_.Remove(values_.Count() - 1);

        size_t count = values_.Count();

        int idx = 0;
        while (true)
        {
            int left = 2 * idx + 1;
            int right = 2 * idx + 2;

            int lowest = idx;

            if (left < count && values_[left] < values_[lowest])
                lowest = left;
            
            if (right < count && values_[right] < values_[lowest])
                lowest = right;

            if (lowest == idx)
                break;

            Swap(values_[lowest], values_[idx]);
            
            idx = lowest;
        }

        return ret;
    }

private:
    Array<int> values_;
};

