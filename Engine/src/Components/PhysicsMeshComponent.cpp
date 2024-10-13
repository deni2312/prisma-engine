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

    ComponentType componentDynamic;
    componentDynamic = std::make_tuple(Prisma::Component::TYPES::BOOL, "Dynamic", &m_collisionData.dynamic);

    ComponentType componentButton;
    m_apply=[&](){
        if (m_collisionData.collider!= static_cast<Prisma::Physics::Collider>(m_status.currentitem)) {
            m_collisionData.collider = static_cast<Prisma::Physics::Collider>(m_status.currentitem);
            colliderDispatcher();
        }
        updateCollisionData();
    };
    componentButton=std::make_tuple(Prisma::Component::TYPES::BUTTON,"Apply Collider",&m_apply);

    addGlobal(componentType);

    addGlobal(componentMass);

    addGlobal(componentDynamic);

    addGlobal(componentButton);
    colliderDispatcher();
    updateCollisionData();
}

void Prisma::PhysicsMeshComponent::update() {

}

void Prisma::PhysicsMeshComponent::destroy()
{

}

void Prisma::PhysicsMeshComponent::collisionData(Prisma::Physics::CollisionData collisionData) {
    m_collisionData = collisionData;
    updateCollisionData();
}

void Prisma::PhysicsMeshComponent::updateCollisionData() {
    if (m_initPhysics) {
        colliderDispatcher();
    }
}

void Prisma::PhysicsMeshComponent::colliderDispatcher() {
    auto mesh = dynamic_cast<Prisma::Mesh*>(parent());
    if (mesh) {
        auto aabbData = mesh->aabbData();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;

        auto isAnimate = dynamic_cast<AnimatedMesh*>(mesh);

        glm::decompose(mesh->parent()->matrix(), scale, rotation, translation, skew, perspective);
        Shape* shape = getShape(scale);

        BodyCreationSettings aabbSettings(shape, Prisma::JtoVec3(translation), Prisma::JtoQuat(rotation), m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static, m_collisionData.dynamic ? Prisma::Layers::MOVING : Prisma::Layers::NON_MOVING);

        JPH::MassProperties mass;
        mass.ScaleToMass(m_collisionData.mass);

        aabbSettings.mMassPropertiesOverride = mass;
        aabbSettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        if (!m_initPhysics) {
            m_physicsId = Prisma::Physics::getInstance().bodyInterface().CreateAndAddBody(aabbSettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
            aabbSettings.mUserData = mesh->uuid();

            m_initPhysics = true;
        }
        else {
            Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(m_physicsId);
            Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(m_physicsId);
            m_physicsId = Prisma::Physics::getInstance().bodyInterface().CreateAndAddBody(aabbSettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
        }
    }
}

Prisma::Physics::CollisionData Prisma::PhysicsMeshComponent::collisionData() {
    return m_collisionData;
}

void Prisma::PhysicsMeshComponent::start() {
    Prisma::Component::start();
    colliderDispatcher();
    updateCollisionData();
}

BodyID Prisma::PhysicsMeshComponent::physicsId() {
    return m_physicsId;
}

bool Prisma::PhysicsMeshComponent::initPhysics() {
    return m_initPhysics;
}

Shape* Prisma::PhysicsMeshComponent::getShape(glm::vec3 scale) {
    auto aabbData = dynamic_cast<Prisma::Mesh*>(parent())->aabbData();

    Shape* shape = nullptr;

    switch (m_collisionData.collider) {
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
    return shape;
}

Prisma::PhysicsMeshComponent::PhysicsMeshComponent() : Prisma::Component{} {
    name("Physics");
}
