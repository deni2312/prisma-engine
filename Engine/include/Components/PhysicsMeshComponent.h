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
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include "Jolt/Physics/SoftBody/SoftBodySharedSettings.h"
// All Jolt symbols are in the JPH namespace
using namespace JPH;

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

namespace Prisma
{
	class PhysicsMeshComponent : public Component
	{
	public:
		PhysicsMeshComponent();

		void ui() override;

		void update() override;

		void destroy() override;

		void onParent(std::shared_ptr<Node> parent) override;

		void collisionData(Physics::CollisionData collisionData);

		void updateCollisionData();

		Physics::CollisionData collisionData();

		void start() override;

		BodyID& physicsId();

		bool initPhysics();

		void onCollisionEnter(std::function<void(const Body&)> add);

		void onCollisionStay(std::function<void(const Body&)> stay);

		void onCollisionExit(std::function<void(const BodyID&)> remove);

		std::function<void(const Body&)> onCollisionEnter();

		std::function<void(const Body&)> onCollisionStay();

		std::function<void(const BodyID&)> onCollisionExit();

		void landscapeData(const Physics::LandscapeData& landscapeData);

		void settingsSoftBody(Prisma::Physics::SoftBodySettings settingsSoft);

		Prisma::Physics::SoftBodySettings settingsSoftBody();

		Body* softId();

		nlohmann::json serialize() override;

		void deserialize(nlohmann::json& data) override;

		glm::vec3 scale() const;

	private:
		ComponentList m_status;
		std::function<void()> m_apply;
		std::function<void()> m_applySoft;
		Physics::CollisionData m_collisionData{};
		void colliderDispatcher();
		BodyCreationSettings getBodySettings();
		void addSoftBody();

		std::shared_ptr<BodyID> m_physicsId = nullptr;
		Body* m_physicsSoftId = nullptr;

		nlohmann::json m_jsonComponent;


		Physics::LandscapeData m_landscapeData;

		const float m_minScale = 0.001;
		const float m_minSize = 0.1;

		Ref<JPH::SoftBodySharedSettings> m_softBodySharedSettings = nullptr;

		std::function<void(const Body&)> m_add = nullptr;
		std::function<void(const Body&)> m_stay = nullptr;
		std::function<void(const BodyID&)> m_remove = nullptr;

		Prisma::Physics::SoftBodySettings m_settingsSoft;
		glm::vec3 m_scale = glm::vec3(1, 1, 1);
	};
}
