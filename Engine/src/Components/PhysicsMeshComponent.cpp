#include "../../include/Components/PhysicsMeshComponent.h"

void Prisma::PhysicsMeshComponent::ui() {
    ComponentType componentType;
    m_status.currentitem=static_cast<int>(m_collisionData.collider);
    m_status.items.push_back("BOX COLLIDER");
    m_status.items.push_back("SPHERE COLLIDER");
    m_status.items.push_back("LANDSCAPE COLLIDER");
    m_status.items.push_back("CONVEX COLLIDER");
    componentType=std::make_tuple(Prisma::Component::TYPES::STRINGLIST,"Collider",&m_status);

    ComponentType componentMass;
    componentMass=std::make_tuple(Prisma::Component::TYPES::FLOAT,"Mass",&m_collisionData.mass);


    ComponentType componentButton;
    m_apply=[&](){
        if (m_collisionData.collider!= static_cast<Prisma::Physics::Collider>(m_status.currentitem)) {
            m_collisionData.collider = static_cast<Prisma::Physics::Collider>(m_status.currentitem);
            colliderDispatcher(m_collisionData.collider);
        }
        updateCollisionData();
        Prisma::CacheScene::getInstance().skipUpdate(true);
    };
    componentButton=std::make_tuple(Prisma::Component::TYPES::BUTTON,"Apply Collider",&m_apply);

    addGlobal(componentType);

    addGlobal(componentMass);

    addGlobal(componentButton);
    colliderDispatcher(m_collisionData.collider);
    updateCollisionData();
}

void Prisma::PhysicsMeshComponent::update() {

}

void Prisma::PhysicsMeshComponent::destroy()
{
    auto physicsWorld = Prisma::Physics::getInstance().physicsWorld();
    if (m_shape && m_body) {
        physicsWorld->collisionShapes.remove(m_shape);
        physicsWorld->dynamicsWorld->removeRigidBody(m_body);
    }
}

void Prisma::PhysicsMeshComponent::collisionData(Prisma::Physics::CollisionData collisionData) {
    m_collisionData = collisionData;
}

void Prisma::PhysicsMeshComponent::updateCollisionData() {
    auto mesh = dynamic_cast<Prisma::Mesh*>(parent());
    auto aabbData = mesh->aabbData();
    auto physicsWorld = Prisma::Physics::getInstance().physicsWorld();

    if (m_shape && m_body) {
        physicsWorld->collisionShapes.remove(m_shape);
        physicsWorld->dynamicsWorld->removeRigidBody(m_body);
    }
    colliderDispatcher(m_collisionData.collider);
    glm::vec3 halfExtents = (aabbData.max - aabbData.min) * 0.5f;

    bool isDynamic = (m_collisionData.mass != 0.f);
    if (isDynamic) {
        m_shape->calculateLocalInertia(m_collisionData.mass, m_collisionData.localInertia);
    }
    btTransform transform;
    transform.setIdentity();
    glm::vec3 origin(aabbData.min.x + halfExtents.x, aabbData.min.y + halfExtents.y,
                     aabbData.min.z + halfExtents.z);

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    auto isAnimate = dynamic_cast<AnimatedMesh*>(mesh);

    glm::decompose(mesh->parent()->matrix(), scale, rotation, translation, skew, perspective);
    origin = glm::vec4(translation + origin, 1.0);

    transform.setFromOpenGLMatrix(glm::value_ptr(mesh->parent()->matrix() * glm::inverse(glm::scale(glm::mat4(1.0f), scale))));

    m_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));

    if (m_collisionData.rigidbody) {
        //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        auto *myMotionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(m_collisionData.mass, myMotionState, m_shape,
                                                        m_collisionData.localInertia);
        m_body = new btRigidBody(rbInfo);

        m_body->setWorldTransform(transform);

        m_body->setUserPointer(mesh);

        physicsWorld->dynamicsWorld->addRigidBody(m_body);
    }
    physicsWorld->collisionShapes.push_back(m_shape);
}

void Prisma::PhysicsMeshComponent::colliderDispatcher(Prisma::Physics::Collider collider) {
    auto mesh = dynamic_cast<Prisma::Mesh*>(parent());
    auto aabbData = mesh->aabbData();
    switch (collider) {
        case Prisma::Physics::Collider::BOX_COLLIDER: {
            glm::vec3 halfExtents = (aabbData.max - aabbData.min) * 0.5f;
            m_shape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
            break;
        }
        case Prisma::Physics::Collider::SPHERE_COLLIDER: {
            glm::vec3 halfExtents = (aabbData.max - aabbData.min) * 0.5f;
            halfExtents.x = 0;
            halfExtents.y = 0;
            float length = glm::length(halfExtents);
            m_shape = new btSphereShape(length);
            break;
        }
        case Prisma::Physics::Collider::LANDSCAPE_COLLIDER: {
            glm::vec3 halfExtents = (aabbData.max - aabbData.min) * 0.5f;

            btTriangleMesh* terrainMesh = new btTriangleMesh();

            auto vertices = mesh->verticesData();


            for (int i = 0; i < vertices.indices.size(); i=i+3) {
                terrainMesh->addTriangle(getVec3BT(vertices.vertices[vertices.indices[i]].position), getVec3BT(vertices.vertices[vertices.indices[i+1]].position), getVec3BT(vertices.vertices[vertices.indices[i+2]].position));
            }
            m_shape = new btBvhTriangleMeshShape(terrainMesh,true);
            break;
        }

        case Prisma::Physics::Collider::CONVEX_COLLIDER: {
            glm::vec3 halfExtents = (aabbData.max - aabbData.min) * 0.5f;

            auto vertices = mesh->verticesData();
            
            m_shape = new btConvexHullShape();

            for (int i = 0; i < vertices.indices.size(); i = i + 3) {
                static_cast<btConvexHullShape*>(m_shape)->addPoint(getVec3BT(vertices.vertices[vertices.indices[i]].position));
                static_cast<btConvexHullShape*>(m_shape)->addPoint(getVec3BT(vertices.vertices[vertices.indices[i+1]].position));
                static_cast<btConvexHullShape*>(m_shape)->addPoint(getVec3BT(vertices.vertices[vertices.indices[i + 2]].position));
            }
            break;
        }

    }
}

Prisma::Physics::CollisionData Prisma::PhysicsMeshComponent::collisionData() {
    return m_collisionData;
}

btRigidBody* Prisma::PhysicsMeshComponent::rigidBody() {
    return m_body;
}

btCollisionShape* Prisma::PhysicsMeshComponent::shape() {
    return m_shape;
}

Prisma::PhysicsMeshComponent::PhysicsMeshComponent() : Prisma::Component{} {
    name("Physics");
}
