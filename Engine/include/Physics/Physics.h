#pragma once

#include "glm/glm.hpp"
#include <memory>
#include "../GlobalData/InstanceData.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#ifdef JPH_DEBUG_RENDERER
#include "DrawDebugger.h"
#endif
#include "Jolt/Physics/SoftBody/SoftBodySharedSettings.h"


namespace Prisma {
class PhysicsMeshComponent;

class Physics : public InstanceData<Physics> {
public:
    enum Collider {
        BOX_COLLIDER,
        SPHERE_COLLIDER,
        LANDSCAPE_COLLIDER,
        CONVEX_COLLIDER
    };

    struct CollisionData {
        Collider collider = BOX_COLLIDER;
        float mass = 0.0f;
        bool dynamic = false;
        bool softBody = false;
        float friction = 0;
        float restitution = 0;
        float pressure = 0;
    };

    struct SoftBodySettings {
        int numIteration = 1.0f;
        bool sleep = false;
        bool updatePosition = false;
        JPH::SoftBodySharedSettings::VertexAttributes vertexAttributes = {1.0e-5f, 1.0e-5f, 1.0e-5f};
        std::vector<std::pair<glm::vec3, float>> customVertices;
    };

    struct LandscapeData {
        JPH::Array<float> landscape;
        JPH::Vec3 offset;
        JPH::Vec3 scale;
        float width;
    };

    Physics();
    void update(float delta);

    JPH::BodyInterface& bodyInterface();

    JPH::PhysicsSystem& physicsSystem();

    void drawDebug();

    void debug(bool debug);

    bool debug();

    void destroy();

private:
    void softBody(std::shared_ptr<PhysicsMeshComponent> physics);
#ifdef JPH_DEBUG_RENDERER
    DrawDebugger* m_drawDebugger;
#endif
    bool m_debug;
    unsigned int m_indexVbo = 0;
};
}