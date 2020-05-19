#pragma once

#include "Types.h"
#include "ps_Math.h"

#include <cassert>


//------------------------------------------------------------------------------
template<class T>
class Array
{
public:
    //------------------------------------------------------------------------------
    Array() = default;

    //------------------------------------------------------------------------------
    ~Array()
    {
        capacity_ = 0;
        count_ = 0;
        delete[] items_;
    }

    //------------------------------------------------------------------------------
    size_t Count() const
    {
        return count_;
    }

    //------------------------------------------------------------------------------
    const T& operator[](size_t index) const
    {
        assert(index < count_);
        return items_[index];
    }

    //------------------------------------------------------------------------------
    T& operator[](size_t index)
    {
        assert(index < count_);
        return items_[index];
    }

    //------------------------------------------------------------------------------
    void Add(const T& item)
    {
        if (count_ >= capacity_)
        {
            auto oldCapacity = capacity_;
            capacity_ = Max(capacity_ << 1, MIN_CAPACITY);
            
            T* newItems = (T*)malloc(sizeof(T) * capacity_);
            memcpy(newItems, items_, sizeof(T) * oldCapacity);
            free(items_);
            items_ = newItems;
        }

        items_[count_++] = item;
    }

    //------------------------------------------------------------------------------
    void Insert(size_t index, const T& item)
    {
        assert(index <= count_);

        if (count_ >= capacity_)
        {
            auto oldCapacity = capacity_;
            capacity_ = Max(capacity_ << 1, MIN_CAPACITY);
            
            T* newItems = (T*)malloc(sizeof(T) * capacity_);
            memcpy(newItems, items_, sizeof(T) * index);
            memcpy(&newItems[index + 1], &items_[index], (oldCapacity - index) * sizeof(T));

            free(items_);
            items_ = newItems;
        }
        else
        {
            // Move items by one to the right
            memmove(&items_[index + 1], &items_[index], (count_ - index) * sizeof(T));
        }

        ++count_;
        items_[index] = item;
    }

    //------------------------------------------------------------------------------
    void Remove(size_t index)
    {
        assert(index < count_);
        items_[index].~T();
        
        --count_;
        memmove(&items_[index], &items_[index + 1], (count_ - index) * sizeof(T));
    }

    //------------------------------------------------------------------------------
    void Clear()
    {
        for (size_t i = 0; i < count_; ++i)
        {
            items_[i].~T();
        }
        count_ = 0;
    }

    //------------------------------------------------------------------------------
    const T& Last()
    {
        assert(count_);
        return items_[count_ - 1];
    }

private:
    static constexpr size_t MIN_CAPACITY = 8;

    size_t capacity_{};
    size_t count_{};
    T* items_{};
};