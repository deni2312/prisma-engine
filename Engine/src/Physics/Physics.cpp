#include "../../include/Physics/Physics.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "../../include/Physics/DrawDebugger.h"
#include "glm/gtx/matrix_decompose.hpp"


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
        auto mesh = (Mesh*)obj->getUserPointer();

        auto prismaMatrix = glm::mat4(1.0f);
        trans.getOpenGLMatrix(glm::value_ptr(prismaMatrix));

        auto scaling = glm::vec3(body->getCollisionShape()->getLocalScaling().getX(), body->getCollisionShape()->getLocalScaling().getY(), body->getCollisionShape()->getLocalScaling().getZ());

        prismaMatrix = prismaMatrix * glm::scale(glm::mat4(1.0f), scaling);
        
        const auto& matrix = mesh->parent()->matrix();

        if (!Prisma::mat4Equals(prismaMatrix, matrix)) {
            mesh->parent()->matrix(prismaMatrix);
        }
    }
}

std::shared_ptr<Prisma::Physics::PhysicsWorld> Prisma::Physics::physicsWorld() {
    return m_physicsWorld;
}
