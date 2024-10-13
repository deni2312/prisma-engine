#include "../../include/Physics/Physics.h"
#include "../../include/Physics/PhysicsData.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "../../include/Physics/DrawDebugger.h"
#include "glm/gtx/matrix_decompose.hpp"

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
// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

struct PhysicsWorldJolt {
    Prisma::BPLayerInterfaceImpl broad_phase_layer_interface;
    Prisma::ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    Prisma::ObjectLayerPairFilterImpl object_vs_object_layer_filter;
    PhysicsSystem physics_system;
    Prisma::MyBodyActivationListener body_activation_listener;
    Prisma::MyContactListener contact_listener;
    std::shared_ptr<TempAllocatorImpl> temp_allocator;
    std::shared_ptr<JobSystemThreadPool> job_system;
};

static std::shared_ptr<PhysicsWorldJolt> physicsWorldJolt = nullptr;

Prisma::Physics::Physics() {
    m_physicsWorld=std::make_shared<PhysicsWorld>();
    physicsWorldJolt = std::make_shared<PhysicsWorldJolt>();

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
    physicsWorldJolt->temp_allocator=std::make_shared<TempAllocatorImpl>(10 * 1024 * 1024);
    physicsWorldJolt->job_system= std::make_shared<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;
    physicsWorldJolt->physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, physicsWorldJolt->broad_phase_layer_interface, physicsWorldJolt->object_vs_broadphase_layer_filter, physicsWorldJolt->object_vs_object_layer_filter);
    physicsWorldJolt->physics_system.SetBodyActivationListener(&physicsWorldJolt->body_activation_listener);
    physicsWorldJolt->physics_system.SetContactListener(&physicsWorldJolt->contact_listener);
    physicsWorldJolt->physics_system.OptimizeBroadPhase();

}

void Prisma::Physics::update(float delta) {

    for (const auto& mesh: currentGlobalScene->meshes) {

    }

    BodyInterface& body_interface = physicsWorldJolt->physics_system.GetBodyInterface();
    physicsWorldJolt->physics_system.Update(delta, 1 , &*physicsWorldJolt->temp_allocator, &*physicsWorldJolt->job_system);
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
