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
        }
        updateCollisionData();
    };
    componentButton=std::make_tuple(Prisma::Component::TYPES::BUTTON,"Apply Collider",&m_apply);

    addGlobal(componentType);

    addGlobal(componentMass);

    addGlobal(componentDynamic);

    addGlobal(componentButton);
    updateCollisionData();
}

void Prisma::PhysicsMeshComponent::update() {

}

void Prisma::PhysicsMeshComponent::destroy()
{
    Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(m_physicsId);
    Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(m_physicsId);
    Prisma::Component::destroy();
}

void Prisma::PhysicsMeshComponent::collisionData(Prisma::Physics::CollisionData collisionData) {
    m_collisionData = collisionData;
    updateCollisionData();
}

void Prisma::PhysicsMeshComponent::updateCollisionData() {
    colliderDispatcher();
}

void Prisma::PhysicsMeshComponent::colliderDispatcher() {
    auto mesh = dynamic_cast<Prisma::Mesh*>(parent());
    if (mesh) {
        auto& bodySettings = getBodySettings();
        if (!m_initPhysics) {
            m_physicsId = Prisma::Physics::getInstance().bodyInterface().CreateAndAddBody(bodySettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
            m_initPhysics = true;
        }
        else {
            Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(m_physicsId);
            Prisma::Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(m_physicsId);
            m_physicsId = Prisma::Physics::getInstance().bodyInterface().CreateAndAddBody(bodySettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
        }
    }
}

Prisma::Physics::CollisionData Prisma::PhysicsMeshComponent::collisionData() {
    return m_collisionData;
}

void Prisma::PhysicsMeshComponent::start() {
    Prisma::Component::start();
    if (!m_initPhysics) {
        updateCollisionData();
    }
}

BodyID& Prisma::PhysicsMeshComponent::physicsId() {
    return m_physicsId;
}

bool Prisma::PhysicsMeshComponent::initPhysics() {
    return m_initPhysics;
}

void Prisma::PhysicsMeshComponent::onCollisionEnter(std::function<void(const Body&)> add) {
    m_add = add;
}

void Prisma::PhysicsMeshComponent::onCollisionStay(std::function<void(const Body&)> stay) {
    m_stay = stay;
}

void Prisma::PhysicsMeshComponent::onCollisionExit(std::function<void(const BodyID&)> remove) {
    m_remove = remove;
}

std::function<void(const Body&)> Prisma::PhysicsMeshComponent::onCollisionEnter() {
    return m_add;
}

std::function<void(const Body&)> Prisma::PhysicsMeshComponent::onCollisionStay() {
    return m_stay;
}

std::function<void(const BodyID&)> Prisma::PhysicsMeshComponent::onCollisionExit() {
    return m_remove;
}

void Prisma::PhysicsMeshComponent::landscapeData(const Prisma::Physics::LandscapeData& landscapeData) {
    m_landscapeData = landscapeData;
}

BodyCreationSettings Prisma::PhysicsMeshComponent::getBodySettings() {
    auto mesh = dynamic_cast<Prisma::Mesh*>(parent());
    BodyCreationSettings aabbSettings;
    auto aabbData = mesh->aabbData();
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    auto isAnimate = dynamic_cast<AnimatedMesh*>(mesh);

    glm::decompose(mesh->parent()->matrix(), scale, rotation, translation, skew, perspective);

    Shape* shape = nullptr;
    switch (m_collisionData.collider) {
        case Prisma::Physics::Collider::BOX_COLLIDER: {
            auto length = (aabbData.max - aabbData.min) * 0.5f;
            auto boxShape = new BoxShape(Prisma::JtoVec3(length));
            shape = new ScaledShape(boxShape, Prisma::JtoVec3(scale));
            aabbSettings = BodyCreationSettings(shape, Prisma::JtoVec3(translation), Prisma::JtoQuat(rotation), m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static, m_collisionData.dynamic ? Prisma::Layers::MOVING : Prisma::Layers::NON_MOVING);
            
            break;
        }
        case Prisma::Physics::Collider::SPHERE_COLLIDER: {
            auto sphereShape = new SphereShape(1.0);
            shape = new ScaledShape(sphereShape, Prisma::JtoVec3(scale));
            aabbSettings = BodyCreationSettings(shape, Prisma::JtoVec3(translation), Prisma::JtoQuat(rotation), m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static, m_collisionData.dynamic ? Prisma::Layers::MOVING : Prisma::Layers::NON_MOVING);
            break;
        }
        case Prisma::Physics::Collider::LANDSCAPE_COLLIDER: {
            HeightFieldShapeSettings settings(m_landscapeData.landscape.data(), m_landscapeData.offset, m_landscapeData.scale, m_landscapeData.width);
            shape = StaticCast<HeightFieldShape>(settings.Create().Get());
            aabbSettings = BodyCreationSettings(shape, Prisma::JtoVec3(translation), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
            break;
        }
        case Prisma::Physics::Collider::CONVEX_COLLIDER: {
            auto length = (aabbData.max - aabbData.min) * 0.5f;
            auto boxShape = new BoxShape(Prisma::JtoVec3(length));
            shape = new ScaledShape(boxShape, Prisma::JtoVec3(scale));
            aabbSettings = BodyCreationSettings(shape, Prisma::JtoVec3(translation), Prisma::JtoQuat(rotation), m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static, m_collisionData.dynamic ? Prisma::Layers::MOVING : Prisma::Layers::NON_MOVING);
        }
    }
    if (m_collisionData.mass > 0 && m_collisionData.collider != Prisma::Physics::LANDSCAPE_COLLIDER) {
        JPH::MassProperties mass;
        mass.ScaleToMass(m_collisionData.mass);

        aabbSettings.mMassPropertiesOverride = mass;
        aabbSettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
    }
    aabbSettings.mUserData = uuid();
    return aabbSettings;
}

Prisma::PhysicsMeshComponent::PhysicsMeshComponent() : Prisma::Component{} {
    name("Physics");
}
