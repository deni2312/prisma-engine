#include "Physics/Physics.h"
#include "Physics/PhysicsData.h"
#include "GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "Components/PhysicsMeshComponent.h"
#include "SceneData/MeshIndirect.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/SoftBody/SoftBodyMotionProperties.h"

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
    physicsWorldJolt->temp_allocator = std::make_shared<TempAllocatorImpl>(10 * 1024 * 1024);
    physicsWorldJolt->job_system = std::make_shared<JobSystemThreadPool>(
        cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
    constexpr uint cMaxBodies = 1024;
    constexpr uint cNumBodyMutexes = 0;
    constexpr uint cMaxBodyPairs = 1024;
    constexpr uint cMaxContactConstraints = 1024;
    physicsWorldJolt->physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                                          physicsWorldJolt->broad_phase_layer_interface,
                                          physicsWorldJolt->object_vs_broadphase_layer_filter,
                                          physicsWorldJolt->object_vs_object_layer_filter);
    physicsWorldJolt->physics_system.SetBodyActivationListener(&physicsWorldJolt->body_activation_listener);
    physicsWorldJolt->physics_system.SetContactListener(&physicsWorldJolt->contact_listener);
    physicsWorldJolt->physics_system.OptimizeBroadPhase();
#ifdef JPH_DEBUG_RENDERER
    m_drawDebugger = new DrawDebugger();
#endif
}

void Prisma::Physics::update(float delta) {
    physicsWorldJolt->physics_system.Update(delta, 1, &*physicsWorldJolt->temp_allocator,
                                            &*physicsWorldJolt->job_system);

    m_indexVbo = 0;
    for (const auto& mesh : GlobalData::getInstance().currentGlobalScene()->meshes) {
        auto physicsComponent = std::dynamic_pointer_cast<PhysicsMeshComponent>(
            mesh->components()["Physics"]);
        if (physicsComponent && physicsComponent->initPhysics()) {
            if (physicsComponent->collisionData().softBody) {
                softBody(physicsComponent);
            } else {
                BodyLockRead lock(physicsSystem().GetBodyLockInterface(),
                                  physicsComponent->physicsId());
                if (lock.Succeeded()) {
                    auto& bInterface = lock.GetBody();
                    const auto& matrix = mesh->parent()->matrix();
                    auto id = physicsComponent->physicsId();
                    glm::mat4 prismaMatrix(1.0);
                    prismaMatrix = JfromMat4(bInterface.GetWorldTransform());
                    auto scaledShape = static_cast<const ScaledShape*>(bInterface.GetShape());
                    if (scaledShape) {
                        prismaMatrix = scale(prismaMatrix, physicsComponent->scale());
                        auto inverseMatrix = inverse(mesh->parent()->parent()->finalMatrix());
                        if (!mat4Equals(prismaMatrix, matrix)) {
                            mesh->parent()->matrix(inverseMatrix * prismaMatrix);
                        }
                    }
                }
            }
        }
        m_indexVbo = m_indexVbo + mesh->verticesData().vertices.size();
    }
}

BodyInterface& Prisma::Physics::bodyInterface() {
    return physicsWorldJolt->physics_system.GetBodyInterface();
}

PhysicsSystem& Prisma::Physics::physicsSystem() {
    return physicsWorldJolt->physics_system;
}

void Prisma::Physics::drawDebug() {
#ifdef JPH_DEBUG_RENDERER

    if (m_debug) {
        BodyManager::DrawSettings settings;
        settings.mDrawShape = true;
        physicsWorldJolt->physics_system.DrawBodies(settings, m_drawDebugger);
        for (const auto& mesh : GlobalData::getInstance().currentGlobalScene()->meshes) {
            auto physicsComponent = std::dynamic_pointer_cast<PhysicsMeshComponent>(mesh->components()["Physics"]);
            if (physicsComponent && physicsComponent->initPhysics()) {
                if (physicsComponent->collisionData().softBody) {
                    auto* softId = physicsComponent->softId();
                    if (softId) {
                        auto mp = static_cast<SoftBodyMotionProperties*>(softId->
                            GetMotionProperties());
                        mp->DrawVertices(m_drawDebugger, Mat44::sIdentity());
                    }
                }
            }
        }
    }
#endif
}

void Prisma::Physics::debug(bool debug) {
    m_debug = debug;
}

bool Prisma::Physics::debug() {
    if (m_debug) {
#ifdef JPH_DEBUG_RENDERER
        m_drawDebugger->init();
#endif
    }
    return m_debug;
}

void Prisma::Physics::destroy() {
#ifdef JPH_DEBUG_RENDERER
    delete m_drawDebugger;
#endif

    delete Factory::sInstance;
}

void Prisma::Physics::softBody(std::shared_ptr<PhysicsMeshComponent> physics) {
    auto mesh = std::dynamic_pointer_cast<Mesh>(physics->parent());
    auto softId = physics->softId();
    if (softId) {
        BodyLockRead lock(physicsSystem().GetBodyLockInterface(), softId->GetID());
        if (lock.Succeeded()) {
            auto mp = static_cast<SoftBodyMotionProperties*>(softId->GetMotionProperties());
            auto& faces = mp->GetFaces();
            auto& verticesSoft = mp->GetVertices();

            auto& verticesData = mesh->verticesData();

            auto& verticesIndirect = MeshIndirect::getInstance().verticesData();

            for (int i = 0; i < verticesData.vertices.size(); i++) {
                verticesData.vertices[i].position = JfromVec3(verticesSoft[i].mPosition);
                verticesIndirect.vertices[m_indexVbo + i].position = verticesData.vertices[i].position;
            }
            auto& contextData = PrismaFunc::getInstance().contextData();

            auto vertexIndirect = MeshIndirect::getInstance().vertexBuffer();
            contextData.immediateContext->UpdateBuffer(vertexIndirect, m_indexVbo * sizeof(Mesh::Vertex), verticesData.vertices.size() * sizeof(Mesh::Vertex), verticesData.vertices.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }
}