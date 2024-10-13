#include "../../include/Physics/Physics.h"
#include "../../include/Physics/PhysicsData.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "../../include/Physics/DrawDebugger.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../../include/Components/PhysicsMeshComponent.h"

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
    physicsWorldJolt = std::make_shared<PhysicsWorldJolt>();
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
    auto& bInterface = physicsWorldJolt->physics_system.GetBodyInterface();

    for (const auto& mesh: currentGlobalScene->meshes) {
        auto physicsComponent = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(mesh->components()["Physics"]);
        if (physicsComponent && physicsComponent->initPhysics()) {

            const auto& matrix = mesh->parent()->matrix();

            auto id = physicsComponent->physicsId();

            BodyLockWrite lock(physicsWorldJolt->physics_system.GetBodyLockInterface(), id);
            glm::mat4 prismaMatrix(1.0);

            if (lock.Succeeded()) {
                auto& body = lock.GetBody();
                prismaMatrix = Prisma::JfromMat4(lock.GetBody().GetWorldTransform());
                const ScaledShape* scaledShape = static_cast<const ScaledShape*>(body.GetShape());
                if (scaledShape) {
                    prismaMatrix = glm::scale(prismaMatrix, Prisma::JfromVec3(scaledShape->GetScale()));
                }
            }
            if (!Prisma::mat4Equals(prismaMatrix, matrix)) {
                mesh->parent()->matrix(prismaMatrix);
            }
        }
    }
    physicsWorldJolt->physics_system.Update(delta, 1 , &*physicsWorldJolt->temp_allocator, &*physicsWorldJolt->job_system);
}

BodyInterface& Prisma::Physics::bodyInterface() {
    return physicsWorldJolt->physics_system.GetBodyInterface();
}

JPH::PhysicsSystem& Prisma::Physics::physicsSystem() {
    return physicsWorldJolt->physics_system;
}
