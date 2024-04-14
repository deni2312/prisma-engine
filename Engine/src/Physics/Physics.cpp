#include "../../include/Physics/Physics.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "../../include/Physics/DrawDebugger.h"
#include "glm/gtx/matrix_decompose.hpp"

std::shared_ptr<Prisma::Physics> Prisma::Physics::instance = nullptr;

Prisma::Physics &Prisma::Physics::getInstance() {
    if (!instance) {
        instance = std::make_shared<Physics>();
    }
    return *instance;
}

Prisma::Physics::Physics() {
    m_physicsWorld=std::make_shared<PhysicsWorld>();
    m_physicsWorld->collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    m_physicsWorld->dispatcher = new btCollisionDispatcher(m_physicsWorld->collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    m_physicsWorld->overlappingPairCache = new btDbvtBroadphase();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    m_physicsWorld->solver = new btSequentialImpulseConstraintSolver;

    m_physicsWorld->dynamicsWorld = new btDiscreteDynamicsWorld(m_physicsWorld->dispatcher, m_physicsWorld->overlappingPairCache, m_physicsWorld->solver, m_physicsWorld->collisionConfiguration);

}

void Prisma::Physics::update(float delta) {
    auto meshes=currentGlobalScene->meshes;
    m_physicsWorld->dynamicsWorld->stepSimulation(delta, 10);
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
        auto mesh=(Mesh*)obj->getUserPointer();
        auto PrismaMatrix=glm::mat4(1.0f);
        trans.getOpenGLMatrix(glm::value_ptr(PrismaMatrix));
        glm::vec3 extents = (mesh->aabbData().max - mesh->aabbData().min)*0.5f;

        auto scaling=glm::vec3(body->getCollisionShape()->getLocalScaling().getX(),body->getCollisionShape()->getLocalScaling().getY(),body->getCollisionShape()->getLocalScaling().getZ());
        glm::vec3 origin(mesh->aabbData().min.x + extents.x, mesh->aabbData().min.y + extents.y,
                         mesh->aabbData().min.z + extents.z);
        glm::mat4 trans_to_pivot   = glm::translate(glm::mat4(1.0f), -(glm::vec3(PrismaMatrix[3])+origin));
        glm::mat4 trans_from_pivot = glm::translate(glm::mat4(1.0f), glm::vec3(PrismaMatrix[3])+origin);
        glm::mat4 rotate_matrix=glm::mat3(PrismaMatrix);

        glm::mat4 resultRotation=trans_from_pivot * rotate_matrix * trans_to_pivot;

        auto scalingVector=PrismaMatrix[3]*(glm::vec4(scaling-glm::vec3(1.0f),1.0));
        PrismaMatrix=glm::scale(glm::mat4(1.0f),scaling)*resultRotation*PrismaMatrix;
        PrismaMatrix[3]=PrismaMatrix[3]-scalingVector;
        PrismaMatrix[3][3]=1;

        PrismaMatrix=glm::translate(PrismaMatrix,glm::vec3(-mesh->aabbData().min.x-extents.x,-mesh->aabbData().min.y-extents.y,-mesh->aabbData().min.z-extents.z));

        mesh->finalMatrix(PrismaMatrix);
    }
}

std::shared_ptr<Prisma::Physics::PhysicsWorld> Prisma::Physics::physicsWorld() {
    return m_physicsWorld;
}
