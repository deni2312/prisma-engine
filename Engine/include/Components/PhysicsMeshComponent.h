#pragma once

#include "Component.h"
#include "../Physics/Physics.h"
#include "glm/detail/type_quat.hpp"
#include "../SceneObjects/Mesh.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../GlobalData/GlobalData.h"

namespace Prisma {

    class PhysicsMeshComponent : public Prisma::Component {
    public:

        PhysicsMeshComponent();

        void ui() override;

        void update() override;

        void collisionData(Prisma::Physics::CollisionData collisionData);

        void updateCollisionData();

        Prisma::Physics::CollisionData collisionData();

    private:
        ComponentList m_status;
        std::function<void()> m_apply;
        Prisma::Physics::CollisionData m_collisionData{};
        btCollisionShape *m_shape = nullptr;
        btRigidBody *m_body = nullptr;
        void colliderDispatcher(Prisma::Physics::Collider collider);

    };

}