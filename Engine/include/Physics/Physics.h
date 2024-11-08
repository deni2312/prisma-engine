#pragma once
#define JPH_DEBUG_RENDERER

#include "glm/glm.hpp"
#include <memory>
#include "../GlobalData/InstanceData.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include "DrawDebugger.h"
#include "Jolt/Physics/SoftBody/SoftBodySharedSettings.h"


namespace Prisma
{
	class PhysicsMeshComponent;

	class Physics : public InstanceData<Physics>
	{
	public:
		enum Collider
		{
			BOX_COLLIDER,
			SPHERE_COLLIDER,
			LANDSCAPE_COLLIDER,
			CONVEX_COLLIDER
		};

		struct CollisionData
		{
			Collider collider;
			float mass = 0.0f;
			bool dynamic = false;
			bool softBody = false;
		};

		struct SoftBodySettings
		{
			float gravity = 1.0f;
			bool sleep = false;
			bool updatePosition = false;
			JPH::SoftBodySharedSettings::VertexAttributes vertexAttributes = {1, 1, 1};
			JPH::SoftBodySharedSettings::EBendType bendType = JPH::SoftBodySharedSettings::EBendType::None;
		};

		struct LandscapeData
		{
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

	private:
		void softBody(std::shared_ptr<Prisma::PhysicsMeshComponent> physics);
		DrawDebugger* m_drawDebugger;
		JPH::BodyManager::DrawSettings m_settings;
		bool m_debug;
		unsigned int m_indexVbo = 0;
		unsigned int m_indexEbo = 0;
	};
}
