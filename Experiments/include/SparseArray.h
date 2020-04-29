#pragma once

#include "IntTypes.h"
#include "Math.h"

#include <stdlib.h>
#include <cstring>
#include <cassert>


/*
bitMap          bitSum      elements
0x000000005     0           a[0]
                            a[2]
0x000018001     2           a[32]
                            a[47]
                            a[48]
0x800000000     5           a[95]
*/

//-----------------------------------------------------------------------------
// TODO Add iterators
template<class Item_t>
class SparseArray
{
public:
    //using Item_t = const char*;
    using BitSum_t = uint;
    using BitField_t = uint64;

    //-----------------------------------------------------------------------------
    SparseArray()
    {
        capacity_ = 1 << 5;
        Realloc();

        bitSumsCapacity_ = 1;
        bitSums_ = (BitSum_t*)malloc(sizeof(BitSum_t) * bitSumsCapacity_);
        bitFields_ = (BitField_t*)malloc(sizeof(BitField_t) * bitSumsCapacity_);

        memset(bitSums_, 0, sizeof(BitSum_t) * bitSumsCapacity_);
        memset(bitFields_, 0, sizeof(BitField_t) * bitSumsCapacity_);
    }

    //-----------------------------------------------------------------------------
    ~SparseArray()
    {
        free(items_);
        free(bitSums_);
        free(bitFields_);
    }

    //-----------------------------------------------------------------------------
    Item_t& operator[](uint idx)
    {
        uint iMain = idx >> 6;
        uint iBits = idx & 63;

        BitField_t bitfield = bitFields_[iMain];

        BitField_t mask = ((BitField_t)1 << iBits);
        assert((bitfield & mask) != 0);

        uint offset = bitSums_[iMain];
        mask = mask - 1;
        uint bitOffset = PopCount64(bitfield & mask);

        return items_[offset + bitOffset];
    }

    //-----------------------------------------------------------------------------
    bool Insert(uint idx, Item_t newItem)
    {
        uint iMain = idx >> 6;
        uint iBits = idx & 63;

        if (iMain >= bitSumsCapacity_)
            ReallocSupport(iMain + 1);

        BitField_t* bitfield = &bitFields_[iMain];
        BitField_t mask = ((BitField_t)1 << iBits);

        // Already present
        if ((*bitfield & mask) != 0)
            return false;
        
        if (count_ + 1 > capacity_)
        {
            capacity_ = capacity_ << 1;
            Realloc();
        }

        ++count_;

        *bitfield |= mask; // Add the element to the bitfield

        for (uint i = iMain + 1; i < bitSumsCapacity_; ++i)
            ++bitSums_[i];

        uint offset = bitSums_[iMain];
        mask = mask - 1;
        uint bitOffset = PopCount64((*bitfield) & mask);
        uint iItem = offset + bitOffset;

        // Shift all items
        for (uint i = count_ - 1; i > iItem; --i)
            items_[i] = items_[i - 1];

        items_[iItem] = newItem;

        return true;
    }

    //-----------------------------------------------------------------------------
    bool Remove(uint idx)
    {
        uint iMain = idx >> 6;
        uint iBits = idx & 63;

        BitField_t* bitfield = &bitFields_[iMain];

        BitField_t mask = ((BitField_t)1 << iBits);
        if ((*bitfield & mask) == 0)
            return false;

        --count_;

        *bitfield &= ~mask; // Remove the element from the bitfield

        for (uint i = iMain + 1; i < bitSumsCapacity_; ++i)
            --bitSums_[i];

        uint offset = bitSums_[iMain];
        mask = mask - 1;
        uint bitOffset = PopCount64((*bitfield) & mask);
        uint iItem = offset + bitOffset;

        // Shift all items
        for (uint i = iItem; i < count_; ++i)
            items_[i] = items_[i + 1];

        //TODO call destructor for items_[count_];

        return true;
    }

private:
    Item_t* items_{};
    BitSum_t* bitSums_{};
    BitField_t* bitFields_{};

    uint count_{};
    uint capacity_{};
    uint bitSumsCapacity_{};

    //-----------------------------------------------------------------------------
    void Realloc()
    {
        void* newItems = malloc(sizeof(Item_t) * capacity_);
        memcpy(newItems, items_, sizeof(Item_t) * count_);
        free(items_);
        items_ = (Item_t*)newItems;
    }

    //-----------------------------------------------------------------------------
    void ReallocSupport(uint newCapacity)
    {
        void* newBitSums = malloc(sizeof(BitSum_t) * newCapacity);
        void* newBitFields = malloc(sizeof(BitField_t) * newCapacity);

        memset(newBitSums, 0, sizeof(BitSum_t) * newCapacity);
        memset(newBitFields, 0, sizeof(BitField_t) * newCapacity);

        memcpy(newBitSums, bitSums_, sizeof(BitSum_t) * bitSumsCapacity_);
        memcpy(newBitFields, bitFields_, sizeof(BitField_t) * bitSumsCapacity_);

        free(bitSums_);
        free(bitFields_);

        bitSums_ = (BitSum_t*)newBitSums;
        bitFields_ = (BitField_t*)newBitFields;

        bitSums_[bitSumsCapacity_] = bitSums_[bitSumsCapacity_ - 1] + PopCount64(bitFields_[bitSumsCapacity_ - 1]);
        for (uint i = bitSumsCapacity_ + 1; i < newCapacity; ++i)
            bitSums_[i] = bitSums_[i - 1];

        bitSumsCapacity_ = newCapacity;
    }
};
