#pragma once
#define JPH_DEBUG_RENDERER

#include "glm/glm.hpp"
#include <memory>
#include "../GlobalData/InstanceData.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "DrawDebugger.h"


namespace Prisma {
    class Physics : public InstanceData<Physics>{
    public:

        enum Collider{
            BOX_COLLIDER,
            SPHERE_COLLIDER,
            LANDSCAPE_COLLIDER,
            CONVEX_COLLIDER
        };

        struct CollisionData{
            Collider collider;
            float mass = 0.0f;
            bool dynamic=false;
        };

        Physics();
        void update(float delta);

        JPH::BodyInterface& bodyInterface();

        JPH::PhysicsSystem& physicsSystem();

        void drawDebug();

        void debug(bool debug);

        bool debug();

    private:
        DrawDebugger* m_drawDebugger;
        JPH::BodyManager::DrawSettings m_settings;
        bool m_debug;

    };
}
