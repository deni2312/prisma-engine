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
    m_drawDebugger = new DrawDebugger();
}

void Prisma::Physics::update(float delta) {
    physicsWorldJolt->physics_system.Update(delta, 10, &*physicsWorldJolt->temp_allocator, &*physicsWorldJolt->job_system);

    auto& bInterface = physicsWorldJolt->physics_system.GetBodyInterface();
    for (const auto& mesh: currentGlobalScene->meshes) {
        auto physicsComponent = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(mesh->components()["Physics"]);
        if (physicsComponent && physicsComponent->initPhysics()) {

            const auto& matrix = mesh->parent()->matrix();

            auto id = physicsComponent->physicsId();

            glm::mat4 prismaMatrix(1.0);
            prismaMatrix = Prisma::JfromMat4(bInterface.GetWorldTransform(id));
            const ScaledShape* scaledShape = static_cast<const ScaledShape*>(bInterface.GetShape(id).GetPtr());
            if (scaledShape) {
                prismaMatrix = glm::scale(prismaMatrix, Prisma::JfromVec3(scaledShape->GetScale()));
            }
            if (!Prisma::mat4Equals(prismaMatrix, matrix)) {
                mesh->parent()->matrix(prismaMatrix);
            }
        }
    }
}

BodyInterface& Prisma::Physics::bodyInterface() {
    return physicsWorldJolt->physics_system.GetBodyInterface();
}

JPH::PhysicsSystem& Prisma::Physics::physicsSystem() {
    return physicsWorldJolt->physics_system;
}

void Prisma::Physics::drawDebug()
{
    if (m_debug) {
        m_drawDebugger->line.setMVP(currentProjection * currentGlobalScene->camera->matrix());
        m_settings.mDrawShape = true;
        physicsWorldJolt->physics_system.DrawBodies(m_settings, m_drawDebugger);
    }
}

void Prisma::Physics::debug(bool debug) {
    m_debug = debug;
}

bool Prisma::Physics::debug() {
    if (m_debug) {
        m_drawDebugger->init();
    }
    return m_debug;
}
