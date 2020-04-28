#include <inttypes.h>
#include <cstdlib>
#include <cstring>

using uint8 = uint8_t;
using uint = uint32_t;
using uint64 = uint64_t;

uint PopCount(uint x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

/*
bitMap          bitSum      elements
0x000000005     0           a[0]
                            a[2]
0x000018001     2           a[32]
                            a[47]
                            a[48]
0x800000000     5           a[95]
*/

//template<class T>
class SparseArray
{
public:
    using Element_t = const char*;
    using BitSum_t = uint8;

    SparseArray()
    {
        capacity_ = 1 << 5;
        Realloc();

        bitSumsCapacity_ = 1;
        bitSums_ = (BitSum_t*)malloc(sizeof(BitSum_t) * bitSumsCapacity_);
        bitField_ = (uint*)malloc(sizeof(uint) * bitSumsCapacity_);

        memset(bitSums_, 0, sizeof(BitSum_t) * bitSumsCapacity_);
        memset(bitField_, 0, sizeof(uint) * bitSumsCapacity_);
    }

    Element_t operator[](uint idx)
    {
        uint iMain = idx >> 5;
        uint iBits = idx & 31;

        uint bitfield = bitField_[iMain];

        uint mask = (1 << iBits);
        if ((bitfield & mask) == 0)
            return nullptr;

        uint offset = bitSums_[iMain];
        mask = mask - 1;
        uint bitOffset = PopCount(bitfield & mask);

        return items_[offset + bitOffset];
    }

    void Insert(uint idx, Element_t newItem)
    {
        uint iMain = idx >> 5;
        uint iBits = idx & 31;

        if (iMain >= bitSumsCapacity_)
            ReallocSupport(iMain + 1);

        uint* bitfield = &bitField_[iMain];
        uint mask = (1 << iBits);

        // Already present
        if ((*bitfield & mask) != 0)
            return;
        
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
        uint bitOffset = PopCount((*bitfield) & mask);
        uint iElem = offset + bitOffset;

        // Shift all items
        for (uint i = count_ - 1; i > iElem; --i)
            items_[i] = items_[i - 1];
    
        items_[iElem] = newItem;
    }

private:
    Element_t* items_{};
    BitSum_t* bitSums_{};
    uint* bitField_{};

    uint count_{};
    uint capacity_{};
    uint bitSumsCapacity_{};

    void Realloc()
    {
        void* newItems = malloc(sizeof(Element_t) * capacity_);
        memcpy(newItems, items_, sizeof(Element_t) * count_);
        free(items_);
        items_ = (Element_t*)newItems;
    }

    void ReallocSupport(uint newCapacity)
    {
        void* newBitSums = malloc(sizeof(BitSum_t) * newCapacity);
        void* newBitFields = malloc(sizeof(uint) * newCapacity);

        memset(newBitSums, 0, sizeof(BitSum_t) * newCapacity);
        memset(newBitFields, 0, sizeof(uint) * newCapacity);

        memcpy(newBitSums, bitSums_, sizeof(BitSum_t) * bitSumsCapacity_);
        memcpy(newBitFields, bitField_, sizeof(uint) * bitSumsCapacity_);

        free(bitSums_);
        free(bitField_);

        bitSums_ = (BitSum_t*)newBitSums;
        bitField_ = (uint*)newBitFields;

        bitSums_[bitSumsCapacity_] = bitSums_[bitSumsCapacity_ - 1] + PopCount(bitField_[bitSumsCapacity_ - 1]);
        for (int i = bitSumsCapacity_ + 1; i < newCapacity; ++i)
            bitSums_[i] = bitSums_[i - 1];

        bitSumsCapacity_ = newCapacity;
    }
};
