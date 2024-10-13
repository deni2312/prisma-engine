#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include "glm/glm.hpp"
#include <memory>
#include "../GlobalData/InstanceData.h"

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include "../../include/Components/PhysicsMeshComponent.h"
// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;


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
            btVector3 localInertia = btVector3(0,0,0);
            bool rigidbody=false;
        };

        struct PhysicsWorld{
            btDefaultCollisionConfiguration* collisionConfiguration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlappingPairCache;
            btSequentialImpulseConstraintSolver* solver;
            btDiscreteDynamicsWorld* dynamicsWorld;
            btAlignedObjectArray<btCollisionShape*> collisionShapes;
        };

        Physics();
        void update(float delta);
        std::shared_ptr<PhysicsWorld> physicsWorld();

        BodyInterface& bodyInterface();
    private:
        std::shared_ptr<PhysicsWorld> m_physicsWorld;
    };
}
