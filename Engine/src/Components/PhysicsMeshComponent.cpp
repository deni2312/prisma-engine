#include "../../include/Components/PhysicsMeshComponent.h"
#include "bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "../../include/Physics/PhysicsData.h"
#include <glm/gtx/string_cast.hpp>

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
    updateCollisionData();
}

void Prisma::PhysicsMeshComponent::updateCollisionData() {

}

void Prisma::PhysicsMeshComponent::colliderDispatcher(Prisma::Physics::Collider collider) {
    auto mesh = dynamic_cast<Prisma::Mesh*>(parent());
    if (mesh && !m_fixed) {
        auto aabbData = mesh->aabbData();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;

        auto isAnimate = dynamic_cast<AnimatedMesh*>(mesh);

        glm::decompose(mesh->parent()->matrix(), scale, rotation, translation, skew, perspective);
        
        auto length = (aabbData.max - aabbData.min) * 0.5f;
        auto boxShape = new BoxShape(Prisma::JtoVec3(length));
        Shape*  shape = nullptr;

        switch (collider) {
            case Prisma::Physics::Collider::BOX_COLLIDER: {
                auto length = (aabbData.max - aabbData.min) * 0.5f;
                auto boxShape = new BoxShape(Prisma::JtoVec3(length));
                shape = new ScaledShape(boxShape, Prisma::JtoVec3(scale));
                break;
            }
            case Prisma::Physics::Collider::SPHERE_COLLIDER: {
                auto lengthSphere = glm::length((aabbData.max - aabbData.min) * 0.5f);
                auto sphereShape = new SphereShape(lengthSphere);
                shape = new ScaledShape(sphereShape, Prisma::JtoVec3(scale));

                break;
            }
            case Prisma::Physics::Collider::LANDSCAPE_COLLIDER: {

                break;
            }

            case Prisma::Physics::Collider::CONVEX_COLLIDER: {

            }
        }

        BodyCreationSettings aabbSettings(shape, Prisma::JtoVec3(translation), Prisma::JtoQuat(rotation), m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static, m_collisionData.dynamic ? Prisma::Layers::MOVING : Prisma::Layers::NON_MOVING);
        m_physicsId = Prisma::Physics::getInstance().bodyInterface().CreateAndAddBody(aabbSettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
        aabbSettings.mUserData = mesh->uuid();
        m_initPhysics = true;
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

void Prisma::PhysicsMeshComponent::fixedRigidBody(bool fixed) {
    m_fixed = fixed;
}

void Prisma::PhysicsMeshComponent::start() {
    Prisma::Component::start();
    colliderDispatcher(m_collisionData.collider);
    updateCollisionData();
}

BodyID Prisma::PhysicsMeshComponent::physicsId() {
    return m_physicsId;
}

bool Prisma::PhysicsMeshComponent::initPhysics() {
    return m_initPhysics;
}

Prisma::PhysicsMeshComponent::PhysicsMeshComponent() : Prisma::Component{} {
    name("Physics");
}
