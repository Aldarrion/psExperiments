#pragma once

#include "Array.h"

#include <cmath>
#include <cassert>

//------------------------------------------------------------------------------
class SortedArray
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
        // Find insert spot
        int bot = 0;
        int top = (int)values_.Count() - 1;
        int idx = (top - bot) / 2;

        while (top > bot)
        {
            if (values_[idx] < x)
            {
                bot = idx + 1;
            }
            else
            {
                top = idx - 1;
            }
            idx = bot + (int)std::ceil((top - bot) / 2);
        }

        if (idx < values_.Count() && x > values_[idx])
            ++idx;

        values_.Insert(idx, x);
    }

    //------------------------------------------------------------------------------
    int RemoveMin()
    {
        int ret = values_[0];

        values_.Remove(0);

        return ret;
    }

private:
    Array<int> values_;
};
