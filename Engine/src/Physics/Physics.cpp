#include "../../include/Physics/Physics.h"
#include "../../include/Physics/PhysicsData.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "../../include/Physics/DrawDebugger.h"
#include "glm/gtx/matrix_decompose.hpp"
// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

Prisma::Physics::Physics() {
    m_physicsWorld=std::make_shared<PhysicsWorld>();
    m_physicsWorldJolt = std::make_shared<PhysicsWorldJolt>();

    m_physicsWorld->collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_physicsWorld->dispatcher = new btCollisionDispatcher(m_physicsWorld->collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    m_physicsWorld->overlappingPairCache = new btDbvtBroadphase();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    m_physicsWorld->solver = new btSequentialImpulseConstraintSolver;

    m_physicsWorld->dynamicsWorld = new btDiscreteDynamicsWorld(m_physicsWorld->dispatcher, m_physicsWorld->overlappingPairCache, m_physicsWorld->solver, m_physicsWorld->collisionConfiguration);


    RegisterDefaultAllocator();
    Factory::sInstance = new Factory();
    RegisterTypes();
    TempAllocatorImpl temp_allocator(10 * 1024 * 1024);
    JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;
    m_physicsWorldJolt->physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_physicsWorldJolt->broad_phase_layer_interface, m_physicsWorldJolt->object_vs_broadphase_layer_filter, m_physicsWorldJolt->object_vs_object_layer_filter);
    m_physicsWorldJolt->physics_system.SetBodyActivationListener(&m_physicsWorldJolt->body_activation_listener);
    m_physicsWorldJolt->physics_system.SetContactListener(&m_physicsWorldJolt->contact_listener);
}

void Prisma::Physics::update(float delta) {
    /*m_physicsWorld->dynamicsWorld->stepSimulation(delta, 10);
    for (int j = m_physicsWorld->dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--)
    {
        btCollisionObject* obj = m_physicsWorld->dynamicsWorld->getCollisionObjectArray()[j];
        btRigidBody* body = btRigidBody::upcast(obj);
        btTransform trans;
        if (body && body->getMotionState())
        {
            body->getMotionState()->getWorldTransform(trans);
        }
        else
        {
            trans = obj->getWorldTransform();
        }
        auto mesh = (Mesh*)obj->getUserPointer();

        auto prismaMatrix = glm::mat4(1.0f);
        trans.getOpenGLMatrix(glm::value_ptr(prismaMatrix));

        auto scaling = glm::vec3(body->getCollisionShape()->getLocalScaling().getX(), body->getCollisionShape()->getLocalScaling().getY(), body->getCollisionShape()->getLocalScaling().getZ());

        prismaMatrix = prismaMatrix * glm::scale(glm::mat4(1.0f), scaling);
        
        const auto& matrix = mesh->parent()->matrix();

        if (!Prisma::mat4Equals(prismaMatrix, matrix)) {
            mesh->parent()->matrix(prismaMatrix);
        }
    }*/


}

std::shared_ptr<Prisma::Physics::PhysicsWorld> Prisma::Physics::physicsWorld() {
    return m_physicsWorld;
}
