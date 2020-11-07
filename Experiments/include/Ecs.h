#pragma once

#include "Types.h"
#include "Array.h"
#include "Span.h"

#include <cstdio>
#include <tuple>
#include <algorithm>

using namespace hs;

namespace archetypeECS
{

#define HS_ALLOCA(Type, count) (Type*)_alloca(count * sizeof(Type))

static constexpr uint64 COMPONENT_COUNT = 100;
static constexpr uint COMPONENT_MASK_SIZE = COMPONENT_COUNT << 6;

using ArchetypeType_t = uint64[COMPONENT_MASK_SIZE];

using Component_t = uint16;
using Entity_t = uint;
using Column_t = void*;


template<class T>
using RemoveCvRef_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<class T>
struct TypeInfo;

struct TypeDetails
{
    uint alignment_;
    uint size_;
};

template<class T>
void Swap(T& a, T& b)
{
    auto tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
}

static constexpr uint ID_BAD{ (uint)-1 };

namespace internal
{

//------------------------------------------------------------------------------
template<class TWithoutCvRef>
struct TypeInfoHelper
{
    inline static uint typeId_{ ID_BAD };
};

}

//------------------------------------------------------------------------------
struct TypeInfoId
{
    template<class T>
    friend struct TypeInfo;

    //------------------------------------------------------------------------------
    static const TypeDetails* GetDetails(uint type)
    {
        return details_[type];
    }

private:
    inline static uint lastTypeId_{ 0 };
    inline static Array<const TypeDetails*> details_;
};

//------------------------------------------------------------------------------
template<class T>
struct TypeInfo
{
    static_assert(std::is_trivial_v<T>);
    static_assert(std::is_standard_layout_v<T>);

    //------------------------------------------------------------------------------
    static uint TypeId()
    {
        const auto id = internal::TypeInfoHelper<RemoveCvRef_t<T>>::typeId_;
        hs_assert(id != ID_BAD && "Type is not registered by TypeInfo<T>::InitTypeId()");
        return id;
    }

    //------------------------------------------------------------------------------
    static void InitTypeId()
    {
        internal::TypeInfoHelper<RemoveCvRef_t<T>>::typeId_ = TypeInfoId::lastTypeId_++;
        details_.alignment_ = alignof(T);
        details_.size_ = sizeof(T);
        TypeInfoId::details_.Add(&details_);

        hs_assert(TypeInfoId::details_.Count() == TypeInfoId::lastTypeId_);
    }

    inline static TypeDetails details_;
};

class EcsWorld;

//------------------------------------------------------------------------------
class Archetype // Table?
{
public:
    using Type_t = Array<uint>;

    //------------------------------------------------------------------------------
    Archetype(EcsWorld* world, const Type_t& type)
        : world_(world)
    {
        type_ = type;
        rowCapacity_ = 8;

        for (int i = 0; i < type_.Count(); ++i)
        {
            // TODO alignment
            const TypeDetails* details = TypeInfoId::GetDetails(type_[i]);
            Column_t column = malloc(rowCapacity_ * details->size_);
            columns_.Add(column);
        }
    }

    //------------------------------------------------------------------------------
    ~Archetype()
    {
        for (int i = 0; i < type_.Count(); ++i)
        {
            free(columns_[i]);
        }
    }


    //------------------------------------------------------------------------------
    Archetype(Archetype&& other) = default;

    //------------------------------------------------------------------------------
    Archetype& operator=(Archetype&& other) = default;

    //------------------------------------------------------------------------------
    Archetype(const Archetype&) = delete;

    //------------------------------------------------------------------------------
    Archetype& operator=(const Archetype&) = delete;

    //------------------------------------------------------------------------------
    const Type_t& GetType() const
    {
        return type_;
    }

    //------------------------------------------------------------------------------
    uint FindComponent(uint componentTypeId) const
    {
        for (int i = 0; i < type_.Count(); ++i)
        {
            if (type_[i] == componentTypeId)
                return i;
        }

        return ID_BAD;
    }

    //------------------------------------------------------------------------------
    template<class TComponent>
    uint FindComponent() const
    {
        auto componentTypeId = TypeInfo<TComponent>::TypeId();
        return FindComponent(componentTypeId);
    }

    //------------------------------------------------------------------------------
    template<class TComponent>
    bool HasComponent() const
    {
        return FindComponent<TComponent>() != ID_BAD;
    }

    //------------------------------------------------------------------------------
    bool IsType(const Type_t& otherType) const
    {
        if (type_.Count() != otherType.Count())
            return false;

        for (int i = 0; i < type_.Count(); ++i)
        {
            if (type_[i] != otherType[i])
                return false;
        }

        return true;
    }

    //------------------------------------------------------------------------------
    void SetComponent(uint rowIdx, uint componentTypeId, void* value)
    {
        hs_assert(componentTypeId != ID_BAD);

        auto size = TypeInfoId::GetDetails(componentTypeId)->size_;

        auto componentIdx = FindComponent(componentTypeId);

        Column_t column = columns_[componentIdx];
        memcpy((byte*)column + rowIdx * size, value, size);
    }

    //------------------------------------------------------------------------------
    template<class TComponent>
    void SetComponent(uint rowIdx, const TComponent& value)
    {
        auto componentId = FindComponent<TComponent>();
        hs_assert(componentId != ID_BAD);
        hs_assert(rowIdx < rowCount_);

        Column_t column = columns_[componentId];
        static_cast<TComponent*>(column)[rowIdx] = value;
    }

    //------------------------------------------------------------------------------
    uint AddEntity(Entity_t eid)
    {
        EnsureCapacity();

        Element entityElement = GetElement(rowCount_, 0);
        memcpy(entityElement.data_, &eid, sizeof(eid));

        for (int i = 1; i < columns_.Count(); ++i)
        {
            Element e = GetElement(rowCount_, i);
            memset(e.data_, 0, e.size_);
        }

        return rowCount_++;
    }

    //------------------------------------------------------------------------------
    void RemoveRow(uint row);

    //------------------------------------------------------------------------------
    int TryGetIterators(const Type_t& canonicalType, Span<const uint> permutation, void** arr) const
    {
        if (canonicalType.Count() > type_.Count())
            return 0;

        int colI = 0;
        for (int i = 0; i < type_.Count(); ++i)
        {
            if (canonicalType[colI] == type_[i])
            {
                auto finalIdx = permutation[colI++];
                arr[finalIdx] = columns_[i];
            }
        }

        if (colI != permutation.Count())
            return 0;

        return rowCount_;
    }

    //------------------------------------------------------------------------------
    struct Element
    {
        void* data_;
        uint size_;
    };

    //------------------------------------------------------------------------------
    Element GetElement(uint row, uint column)
    {
        const TypeDetails* details = TypeInfoId::GetDetails(type_[column]);
        Element element;
        element.data_ = (byte*)columns_[column] + row * details->size_;
        element.size_ = details->size_;
        return element;
    }

private:
    EcsWorld* world_;
    Type_t type_;
    Array<Column_t> columns_;
    uint rowCount_{};
    uint rowCapacity_;

    //------------------------------------------------------------------------------
    uint GetEntityId(uint row)
    {
        auto element = GetElement(row, 0);
        return *(Entity_t*)element.data_;
    }

    //------------------------------------------------------------------------------
    void EnsureCapacity()
    {
        if (rowCount_ == rowCapacity_)
        {
            rowCapacity_ *= 2;
            for (int i = 0; i < columns_.Count(); ++i)
            {
                // TODO alignment
                const TypeDetails* details = TypeInfoId::GetDetails(type_[i]);
                Column_t newColumn = malloc(rowCapacity_ * details->size_);
                memcpy(newColumn, columns_[i], rowCount_ * details->size_);
                free(columns_[i]);
                columns_[i] = newColumn;
            }
        }
    }

    //------------------------------------------------------------------------------
    void SwapRow(uint a, uint b)
    {
        for (int i = 0; i < columns_.Count(); ++i)
        {
            const TypeDetails* details = TypeInfoId::GetDetails(type_[i]);
            auto size = details->size_;

            byte* tmp = HS_ALLOCA(byte, size);
            byte* aPtr = (byte*)columns_[i] + size * a;
            byte* bPtr = (byte*)columns_[i] + size * b;

            memcpy(tmp, aPtr, size);
            memcpy(aPtr, bPtr, size);
            memcpy(bPtr, tmp ,size);
        }
    }
};

//------------------------------------------------------------------------------
// Class that has all the types and entities
class EcsWorld
{
    friend class Archetype;

public:
    //------------------------------------------------------------------------------
    EcsWorld()
    {
        Archetype emptyArchetype(this, { 0 });
        archetypes_.Add(std::move(emptyArchetype));
    }

    //------------------------------------------------------------------------------
    Entity_t CreteEntity()
    {
        Entity_t id{};

        if (denseUsedCount_ == dense_.Count())
        {
            id = denseUsedCount_;
            dense_.Add(id);
            sparse_.Add(id);
        }
        else
        {
            hs_assert(denseUsedCount_ < dense_.Count());
            id = dense_[denseUsedCount_];
            sparse_[id] = denseUsedCount_;
        }
        ++denseUsedCount_;

        EntityRecord record;
        record.archetype_= 0;
        record.rowIndex_ = archetypes_[0].AddEntity(id);

        records_.Add(record);

        return id;
    }

    //------------------------------------------------------------------------------
    void DeleteEntity(Entity_t entity)
    {
        hs_assert(denseUsedCount_ > 0);

        auto denseIdx = sparse_[entity];
        auto lastDense = denseUsedCount_ - 1;
        hs_assert(denseIdx <= denseUsedCount_);

        const auto& record = records_[denseIdx];
        archetypes_[record.archetype_].RemoveRow(record.rowIndex_);

        if (denseIdx < lastDense)
        {
            SwapEntity(denseIdx, lastDense);
        }

        // Remove last entity
        --denseUsedCount_;
    }

    //------------------------------------------------------------------------------
    template<class TComponent>
    void SetComponent(Entity_t entity, const TComponent& component)
    {
        // Find entity
        auto dense = sparse_[entity];
        EntityRecord& record = records_[dense];
        Archetype* originalArch = &archetypes_[record.archetype_];

        if (originalArch->HasComponent<TComponent>())
        {
            originalArch->SetComponent(record.rowIndex_, component);
        }
        else
        {
            // TODO get rid of this allocation
            auto type = originalArch->GetType();
            AddComponentToType<TComponent>(type);

            uint archetypeIdx = ID_BAD;

            for (int i = 0; i < archetypes_.Count(); ++i)
            {
                if (archetypes_[i].IsType(type))
                {
                    archetypeIdx = i;
                    break;
                }
            }

            if (archetypeIdx == ID_BAD)
            {
                Archetype newArchetype(this, type);
                archetypes_.Add(std::move(newArchetype));
                archetypeIdx = archetypes_.Count() - 1;
            }

            hs_assert(archetypeIdx != ID_BAD);

            // Copy components one by one from old to the new originalArch
            Archetype* newArch = &archetypes_[archetypeIdx];

            EntityRecord newRecord;
            newRecord.archetype_ = archetypeIdx;
            newRecord.rowIndex_ = newArch->AddEntity(entity);

            const auto& oldType = originalArch->GetType();

            for (int i = 0; i < oldType.Count(); ++i)
            {
                void* oldValue = originalArch->GetElement(record.rowIndex_, i).data_;
                newArch->SetComponent(newRecord.rowIndex_, oldType[i], oldValue);
            }

            newArch->SetComponent(newRecord.rowIndex_, component);

            originalArch->RemoveRow(record.rowIndex_);

            record = newRecord;
        }
    }

    //------------------------------------------------------------------------------
    void GetEntities(uint*& begin, uint& count)
    {
        begin = dense_.Data();
        count = denseUsedCount_;
    }

    //------------------------------------------------------------------------------
    template<class... TComponents>
    struct Iter
    {
        explicit Iter(EcsWorld* world) : world_(world) {}

        //------------------------------------------------------------------------------
        template<class TFun>
        void Each(TFun fun)
        {
            static constexpr int COMP_COUNT = sizeof...(TComponents);
            auto seq = std::make_index_sequence<COMP_COUNT>();

            Archetype::Type_t type{ TypeInfo<TComponents>::TypeId()... };
            Archetype::Type_t canonicalType = type;
            std::sort(canonicalType.begin(), canonicalType.end());

            uint permutation[COMP_COUNT];
            for (int i = 0; i < COMP_COUNT; ++i)
            {
                permutation[i] = type.IndexOf(canonicalType[i]);
            }

            for (int archI = 0; archI < world_->archetypes_.Count(); ++archI)
            {
                void* arr[COMP_COUNT]{};
                if (int rowCount = world_->archetypes_[archI].TryGetIterators(canonicalType, MakeSpan(permutation), arr);
                    rowCount)
                {
                    for (int rowI = 0; rowI < rowCount; ++rowI)
                    {
                        CallHelper(arr, rowI, fun, seq);
                    }
                }
            }
        }

    private:
        EcsWorld* world_;

        //------------------------------------------------------------------------------
        template<class TFun, unsigned long... Seq>
        void CallHelper(void** arr, int row, TFun fun, std::index_sequence<Seq...>)
        {
            fun(((TComponents*)arr[Seq])[row]...);
        }

    };

private:
    struct EntityRecord
    {
        uint archetype_;
        uint rowIndex_;
    };

    Array<Entity_t>     sparse_;
    Array<uint>         dense_;
    Array<EntityRecord> records_;
    Array<Archetype>    archetypes_;
    uint                denseUsedCount_{};

    //------------------------------------------------------------------------------
    void SwapEntity(uint denseIdxA, uint denseIdxB)
    {
        Swap(dense_[denseIdxA], dense_[denseIdxB]);
        Swap(records_[denseIdxA], records_[denseIdxB]);

        sparse_[dense_[denseIdxA]] = denseIdxA;
        sparse_[dense_[denseIdxB]] = denseIdxB;
    }

    //------------------------------------------------------------------------------
    template<class TComponent>
    void AddComponentToType(Archetype::Type_t& type)
    {
        auto typeId = TypeInfo<TComponent>::TypeId();
        // TODO binary search? But we will change the system to bitfields anyway
        for (int i = 0; i < type.Count(); ++i)
        {
            hs_assert(type[i] != typeId && "TypeId already present in type");
            if (type[i] > typeId)
            {
                type.Insert(i, typeId);
                return;
            }
        }

        type.Add(typeId);
    }

    //------------------------------------------------------------------------------
    void UpdateRecord(Entity_t eid, uint rowIdx)
    {
        auto denseIdx = sparse_[eid];
        records_[denseIdx].rowIndex_ = rowIdx;
    }
};

//------------------------------------------------------------------------------
void Archetype::RemoveRow(uint row)
{
    hs_assert(row < rowCount_);

    const uint lastRowIdx = rowCount_ - 1;
    if (row < lastRowIdx)
    {
        auto eid = GetEntityId(lastRowIdx);
        world_->UpdateRecord(eid, row);
        SwapRow(row, lastRowIdx);
    }

    --rowCount_;
}


class Entity // ? or just have the number and pass the world to all the functions?
{
    // If we have the class we should have the entity API here
private:
    Entity_t id_;
    EcsWorld* world_;
};

struct TransformComponent
{
    float x;
    float y;
};

struct ImageComponent
{
    uint bitmap_[8][8]{};
};

}


namespace simpleECS
{

using Entity_t = uint;
using Component_t = uint16;

static constexpr Entity_t COMPONENT_TYPE_COUNT = 64;
static constexpr Component_t NULL_COMPONENT = (Component_t)-1;

struct Entity
{
    Component_t components_[COMPONENT_TYPE_COUNT];
};

struct TransformComponent
{
    static constexpr Component_t TYPE_ID = 0;

    Entity_t entity_;
    float x;
    float y;
};

struct ImageComponent
{
    static constexpr Component_t TYPE_ID = 1;

    Entity_t entity_;
    uint bitmap_[8][8]{};
};

static Array<Entity> g_Entities;
static Array<TransformComponent> g_Transforms;
static Array<ImageComponent> g_Images;

uint CreateEntity()
{
    uint id = g_Entities.Count();
    g_Entities.Add(Entity());

    for (uint i = 0; i < COMPONENT_TYPE_COUNT; ++i)
        g_Entities.Last().components_[i] = NULL_COMPONENT;

    return id;
}

inline void Init()
{
    // Create some entities
    Entity_t eid = CreateEntity();
    g_Entities[eid].components_[TransformComponent::TYPE_ID] = 0;
    g_Transforms.Add(TransformComponent{ eid, 1, 2 });

    eid = CreateEntity();
    g_Entities[eid].components_[TransformComponent::TYPE_ID] = 1;
    g_Entities[eid].components_[ImageComponent::TYPE_ID] = 0;
    g_Transforms.Add(TransformComponent{ eid, 2, 3 });
    g_Images.Add(ImageComponent{ eid });

    eid = CreateEntity();
    g_Entities[eid].components_[TransformComponent::TYPE_ID] = 2;
    g_Transforms.Add(TransformComponent{ eid, 3, 4 });
}

inline void MovementSystem()
{
    for (uint i = 0; i < g_Transforms.Count(); ++i)
    {
        g_Transforms[i].x += 0.1f;
        g_Transforms[i].y += 0.2f;
    }
}

inline void RenderSystem()
{
    for (uint i = 0; i < g_Images.Count(); ++i)
    {
        const Entity_t ent = g_Images[i].entity_;
        const Component_t transform = g_Entities[ent].components_[TransformComponent::TYPE_ID];
        // This is a potential problem. If we wanted more specific combination of components there could be many ifs like this one.
        // If we had 10000 entities with transform but just 10 with our combination of components, we would have to chose which
        // component array is the smallest and iterate that one while skipping the other ones.
        // Needless to say that when indexing to the other component arrays we have many cache missies.
        // This problem should be solved by entity archetypes, where we would iterate only the archetypes matching our selection.
        if (transform == NULL_COMPONENT)
            continue;

        const TransformComponent& transformComponent = g_Transforms[transform];
        printf("Render at: %f, %f\n", transformComponent.x, transformComponent.y);
    }
}

}

namespace betterECS
{

using EntityType_t = uint64; // Mask of components given entity has, could be multiple uint64s each gives us 64 component types

using Entity_t = uint;
using Component_t = uint16;

static constexpr Entity_t COMPONENT_TYPE_COUNT = 64;
static constexpr Component_t NULL_COMPONENT = (Component_t)-1;

struct TransformComponent
{
    static constexpr Component_t TYPE_ID = 0;

    float x;
    float y;
};

struct ImageComponent
{
    static constexpr Component_t TYPE_ID = 1;

    uint bitmap_[8][8]{};
};

bool HasComponent(Entity_t entity, Component_t component)
{
    EntityType_t type = 0x2; // Get it from somewhere
    return (type & (1 << component));
}

}

