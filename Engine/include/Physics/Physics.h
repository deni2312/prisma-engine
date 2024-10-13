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
#include "PhysicsData.h"

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

        struct PhysicsWorldJolt {
            BPLayerInterfaceImpl broad_phase_layer_interface;
            ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
            ObjectLayerPairFilterImpl object_vs_object_layer_filter;
            PhysicsSystem physics_system;
            MyBodyActivationListener body_activation_listener;
            MyContactListener contact_listener;
        };

        Physics();
        void update(float delta);
        std::shared_ptr<PhysicsWorld> physicsWorld();
    private:
        std::shared_ptr<PhysicsWorld> m_physicsWorld;
        std::shared_ptr<PhysicsWorldJolt> m_physicsWorldJolt;
    };
}
