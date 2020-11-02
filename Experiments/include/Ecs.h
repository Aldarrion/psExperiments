#pragma once

#include "Types.h"
#include "Array.h"

#include <cstdio>
#include <tuple>

using namespace hs;

namespace archetypeECS
{

#define ECS_COMPONENT \
    static uint TYPE_ID;

static constexpr uint64 COMPONENT_COUNT = 100;
static constexpr uint COMPONENT_MASK_SIZE = COMPONENT_COUNT << 6;

using ArchetypeType_t = uint64[COMPONENT_MASK_SIZE];

using Component_t = uint16;
using Entity_t = uint;
using Column_t = void*;

class Archetype // Table?
{
public:


private:
    Array<uint> type_;
    Array<Column_t> columns_;
    uint rowCount_;
    uint rowCapacity_;
};

// Class that has all the types and entities
class EcsWorld
{
public:
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

        // TODO null record

        return id;
    }

    void DeleteEntity(Entity_t entity)
    {
        hs_assert(denseUsedCount_ > 0);

        auto denseIdx = sparse_[entity];
        auto lastDense = denseUsedCount_ - 1;
        hs_assert(denseIdx <= denseUsedCount_);

        if (denseIdx < lastDense)
        {
            SwapEntity(denseIdx, lastDense);
        }

        // Remove last entity
        --denseUsedCount_;
    }

    template<class TComponent>
    void SetComponent(Entity_t entity)
    {
        // Find entity
    }

    void GetEntities(uint*& begin, uint& count)
    {
        begin = dense_.Data();
        count = denseUsedCount_;
    }

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

    void SwapEntity(uint e, uint f)
    {
        auto tmp = dense_[e];
        dense_[e] = dense_[f];
        dense_[f] = tmp;

        sparse_[dense_[e]] = e;
        sparse_[dense_[f]] = f;

        // TODO swap records
    }
};


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

