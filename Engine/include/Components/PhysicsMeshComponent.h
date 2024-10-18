#pragma once

#include "Component.h"
#include "../Physics/Physics.h"
#include "glm/detail/type_quat.hpp"
#include "../SceneObjects/Mesh.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../GlobalData/GlobalData.h"
#include <functional>

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

namespace Prisma {

    class PhysicsMeshComponent : public Prisma::Component {
    public:

        PhysicsMeshComponent();

        void ui() override;

        void update() override;

        virtual void destroy() override;

        void collisionData(Prisma::Physics::CollisionData collisionData);

        void updateCollisionData();

        Prisma::Physics::CollisionData collisionData();

        void start() override;

        BodyID& physicsId();

        bool initPhysics();

        void onCollisionEnter(std::function<void(const Body&)> add);

        void onCollisionStay(std::function<void(const Body&)> stay);

        void onCollisionExit(std::function<void(const BodyID&)> remove);

        std::function<void(const Body&)> onCollisionEnter();

        std::function<void(const Body&)> onCollisionStay();

        std::function<void(const BodyID&)> onCollisionExit();

        void landscapeData(const Prisma::Physics::LandscapeData& landscapeData);

    private:
        ComponentList m_status;
        std::function<void()> m_apply;
        Prisma::Physics::CollisionData m_collisionData{};
        btCollisionShape *m_shape = nullptr;
        btRigidBody *m_body = nullptr;
        void colliderDispatcher();
        Shape* getShape(glm::vec3 translation,glm::vec3 scale);
        BodyID m_physicsId;
        bool m_initPhysics = false;

        Prisma::Physics::LandscapeData m_landscapeData;

        std::function<void(const Body&)> m_add = nullptr;
        std::function<void(const Body&)> m_stay = nullptr;
        std::function<void(const BodyID&)> m_remove = nullptr;
    };

}