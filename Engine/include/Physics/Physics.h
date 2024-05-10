#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include "glm/glm.hpp"
#include <memory>

namespace Prisma {
    class Physics {
    public:

        enum Collider{
            BOX_COLLIDER,
            SPHERE_COLLIDER,
            LANDSCAPE_COLLIDER
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
        static Physics& getInstance();
        void update(float delta);
        std::shared_ptr<PhysicsWorld> physicsWorld();
        Physics(const Physics&) = delete;
        Physics& operator=(const Physics&) = delete;
    private:
        static std::shared_ptr<Physics> instance;
        std::shared_ptr<PhysicsWorld> m_physicsWorld;
    };
}
