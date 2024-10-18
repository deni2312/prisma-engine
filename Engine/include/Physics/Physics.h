#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include "glm/glm.hpp"
#include <memory>
#include "../GlobalData/InstanceData.h"
#define JPH_DEBUG_RENDERER

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
            float mass =0.0f;
            JPH::Vec3 localInertia = JPH::Vec3(0,0,0);
            bool dynamic=false;
        };

        Physics();
        void update(float delta);

        JPH::BodyInterface& bodyInterface();

        JPH::PhysicsSystem& physicsSystem();

        void drawDebug();

    private:
        DrawDebugger* m_drawDebugger;
        JPH::BodyManager::DrawSettings m_settings;

    };
}
