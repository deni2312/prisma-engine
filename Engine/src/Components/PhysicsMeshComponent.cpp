#include "../../include/Components/PhysicsMeshComponent.h"
#include "../../include/Physics/PhysicsData.h"
#include <glm/gtx/string_cast.hpp>

#include "../../include/SceneData/MeshIndirect.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/SoftBody/SoftBodyCreationSettings.h"
#include "../../include/SceneData/ComponentTypeLayout.h"

void Prisma::PhysicsMeshComponent::ui()
{
	Prisma::Component::ui();

	ComponentType componentType;
	m_status.currentitem = static_cast<int>(m_collisionData.collider);
	m_status.items.push_back("BOX COLLIDER");
	m_status.items.push_back("SPHERE COLLIDER");
	m_status.items.push_back("LANDSCAPE COLLIDER");
	m_status.items.push_back("CONVEX COLLIDER");
	componentType = std::make_tuple(TYPES::STRINGLIST, "Collider", &m_status);
	ComponentType componentMass;
	componentMass = std::make_tuple(TYPES::FLOAT, "Mass", &m_collisionData.mass);
	ComponentType componentFriction;
	componentFriction = std::make_tuple(TYPES::FLOAT, "Friction", &m_collisionData.friction);
	ComponentType componentRestitution;
	componentRestitution = std::make_tuple(TYPES::FLOAT, "Restitution", &m_collisionData.restitution);
	ComponentType componentPressure;
	componentPressure = std::make_tuple(TYPES::FLOAT, "Pressure", &m_collisionData.pressure);
	ComponentType componentIteration;
	componentIteration = std::make_tuple(TYPES::INT, "Iteration", &m_settingsSoft.numIteration);
	ComponentType componentDynamic;
	componentDynamic = std::make_tuple(TYPES::BOOL, "Dynamic", &m_collisionData.dynamic);

	ComponentType componentButton;
	m_apply = [&]()
	{
		if (m_collisionData.collider != static_cast<Physics::Collider>(m_status.currentitem))
		{
			m_collisionData.collider = static_cast<Physics::Collider>(m_status.currentitem);
		}
		updateCollisionData();
	};

	m_applySoft = [&]()
	{
		m_collisionData.softBody = true;
		updateCollisionData();
	};

	ComponentType componentSoftBody;
	componentSoftBody = std::make_tuple(TYPES::BUTTON, "Soft Body", &m_applySoft);

	componentButton = std::make_tuple(TYPES::BUTTON, "Apply Collider", &m_apply);

	addGlobal({componentType,false });

	addGlobal({componentMass,false });

	addGlobal({componentFriction,false });

	addGlobal({componentRestitution,false });

	addGlobal({componentPressure,false });

	addGlobal({componentIteration,false });

	addGlobal({componentDynamic,false });

	addGlobal({componentSoftBody,true });

	addGlobal({componentButton,false });

	updateCollisionData();
}

void Prisma::PhysicsMeshComponent::update()
{
}

void Prisma::PhysicsMeshComponent::destroy()
{
	if (m_physicsId)
	{
		Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(*m_physicsId);
		Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(*m_physicsId);
		m_physicsId = nullptr;
	}
	if (m_physicsSoftId)
	{
		Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(m_physicsSoftId->GetID());
		Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(m_physicsSoftId->GetID());
		m_physicsSoftId = nullptr;
	}
	Component::destroy();
}

void Prisma::PhysicsMeshComponent::onParent(Node* parent)
{
	Component::onParent(parent);
	updateCollisionData();
}

void Prisma::PhysicsMeshComponent::collisionData(Physics::CollisionData collisionData)
{
	m_collisionData = collisionData;
	updateCollisionData();
}

void Prisma::PhysicsMeshComponent::updateCollisionData()
{
	colliderDispatcher();
}

void Prisma::PhysicsMeshComponent::colliderDispatcher()
{
	auto mesh = dynamic_cast<Mesh*>(parent());
	if (mesh)
	{
		if (!m_collisionData.softBody)
		{
			auto bodySettings = getBodySettings();
			if (!m_physicsId)
			{
				m_physicsId = std::make_shared<BodyID>();
				*m_physicsId = Physics::getInstance().bodyInterface().CreateAndAddBody(
					bodySettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
			}
			else
			{
				Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(*m_physicsId);
				Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(*m_physicsId);
				*m_physicsId = Physics::getInstance().bodyInterface().CreateAndAddBody(
					bodySettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
			}
		}
		else
		{
			addSoftBody();
		}
	}
}

Prisma::Physics::CollisionData Prisma::PhysicsMeshComponent::collisionData()
{
	return m_collisionData;
}

void Prisma::PhysicsMeshComponent::start()
{
	Component::start();
	if (!m_physicsId)
	{
		updateCollisionData();
	}
}

BodyID& Prisma::PhysicsMeshComponent::physicsId()
{
	return *m_physicsId;
}

bool Prisma::PhysicsMeshComponent::initPhysics()
{
	return static_cast<bool>(m_physicsId) || static_cast<bool>(m_physicsSoftId);
}

void Prisma::PhysicsMeshComponent::onCollisionEnter(std::function<void(const Body&)> add)
{
	m_add = add;
}

void Prisma::PhysicsMeshComponent::onCollisionStay(std::function<void(const Body&)> stay)
{
	m_stay = stay;
}

void Prisma::PhysicsMeshComponent::onCollisionExit(std::function<void(const BodyID&)> remove)
{
	m_remove = remove;
}

std::function<void(const Body&)> Prisma::PhysicsMeshComponent::onCollisionEnter()
{
	return m_add;
}

std::function<void(const Body&)> Prisma::PhysicsMeshComponent::onCollisionStay()
{
	return m_stay;
}

std::function<void(const BodyID&)> Prisma::PhysicsMeshComponent::onCollisionExit()
{
	return m_remove;
}

void Prisma::PhysicsMeshComponent::landscapeData(const Physics::LandscapeData& landscapeData)
{
	m_landscapeData = landscapeData;
}

void Prisma::PhysicsMeshComponent::settingsSoftBody(Prisma::Physics::SoftBodySettings settingsSoft)
{
	m_settingsSoft = settingsSoft;
}

Prisma::Physics::SoftBodySettings Prisma::PhysicsMeshComponent::settingsSoftBody()
{
	return m_settingsSoft;
}

Body* Prisma::PhysicsMeshComponent::softId()
{
	return m_physicsSoftId;
}

nlohmann::json Prisma::PhysicsMeshComponent::serialize()
{
	m_jsonComponent = {
		{"CollisionData", m_collisionData},
		{"LandscapeData", m_landscapeData},
		{"SoftBodySettings", m_settingsSoft}
	};
	return m_jsonComponent;
}

void Prisma::PhysicsMeshComponent::deserialize(nlohmann::json& data)
{
	// Validate input JSON and populate member variables
	if (data.contains("CollisionData") && data["CollisionData"].is_object())
		data.at("CollisionData").get_to(m_collisionData);

	if (data.contains("LandscapeData") && data["LandscapeData"].is_object())
		data.at("LandscapeData").get_to(m_landscapeData);

	if (data.contains("SoftBodySettings") && data["SoftBodySettings"].is_object())
		data.at("SoftBodySettings").get_to(m_settingsSoft);
}

glm::vec3 Prisma::PhysicsMeshComponent::scale() const
{
	return m_scale;
}

BodyCreationSettings Prisma::PhysicsMeshComponent::getBodySettings()
{
	auto mesh = dynamic_cast<Mesh*>(parent());
	BodyCreationSettings aabbSettings;
	auto aabbData = mesh->aabbData();
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	decompose(mesh->parent()->finalMatrix(), scale, rotation, translation, skew, perspective);
	m_scale = scale;
	if (scale.x < m_minScale || scale.y < m_minScale || scale.z < m_minScale)
	{
		scale = glm::vec3(m_minScale);
		translation = glm::vec3(0.0);
		rotation = glm::quat({0, 0, 0});
	}
	Shape* shape = nullptr;
	switch (m_collisionData.collider)
	{
	case Physics::Collider::BOX_COLLIDER:
		{
			auto length = glm::abs((aabbData.max - aabbData.min) * 0.5f);
			if (length.x < m_minScale)
			{
				length.x = m_minSize;
			}

			if (length.y < m_minScale)
			{
				length.y = m_minSize;
			}

			if (length.z < m_minScale)
			{
				length.z = m_minSize;
			}
			auto boxShape = new BoxShape(JtoVec3(length));
			auto result = boxShape->ScaleShape(JtoVec3(scale));
			shape = result.Get();
			aabbSettings = BodyCreationSettings(shape, JtoVec3(translation), JtoQuat(rotation),
			                                    m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static,
			                                    m_collisionData.dynamic
				                                    ? Layers::MOVING
				                                    : Layers::NON_MOVING);

			break;
		}
	case Physics::Collider::SPHERE_COLLIDER:
		{
			auto sphereShape = new SphereShape(1.0);
			auto result = sphereShape->ScaleShape(JtoVec3(scale));
			shape = result.Get();
			aabbSettings = BodyCreationSettings(shape, JtoVec3(translation), JtoQuat(rotation),
			                                    m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static,
			                                    m_collisionData.dynamic
				                                    ? Layers::MOVING
				                                    : Layers::NON_MOVING);
			break;
		}
	case Physics::Collider::LANDSCAPE_COLLIDER:
		{
			HeightFieldShapeSettings settings(m_landscapeData.landscape.data(), m_landscapeData.offset,
			                                  m_landscapeData.scale, m_landscapeData.width);
			shape = StaticCast<HeightFieldShape>(settings.Create().Get());
			aabbSettings = BodyCreationSettings(shape, JtoVec3(translation), JtoQuat(rotation),
			                                    EMotionType::Static, Layers::NON_MOVING);
			break;
		}
	case Physics::Collider::CONVEX_COLLIDER:
		{
			auto length = (aabbData.max - aabbData.min) * 0.5f;
			ConvexHullShapeSettings settings;
			for (auto v : mesh->verticesData().vertices)
			{
				settings.mPoints.push_back(Prisma::JtoVec3(v.position));
			}

			Shape::ShapeResult result;
			auto convexShape = new ConvexHullShape(settings, result);
			auto resultShape = convexShape->ScaleShape(JtoVec3(scale));
			shape = resultShape.Get();
			aabbSettings = BodyCreationSettings(shape, JtoVec3(translation), JtoQuat(rotation),
			                                    m_collisionData.dynamic ? EMotionType::Dynamic : EMotionType::Static,
			                                    m_collisionData.dynamic
				                                    ? Layers::MOVING
				                                    : Layers::NON_MOVING);
		}
	}
	if (m_collisionData.mass > 0 && m_collisionData.collider != Physics::LANDSCAPE_COLLIDER)
	{
		MassProperties mass;
		mass.ScaleToMass(m_collisionData.mass);

		aabbSettings.mMassPropertiesOverride = mass;
		aabbSettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	}
	aabbSettings.mFriction = m_collisionData.friction;
	aabbSettings.mRestitution = m_collisionData.restitution;

	aabbSettings.mUserData = uuid();
	return aabbSettings;
}

void Prisma::PhysicsMeshComponent::addSoftBody()
{
	if (m_physicsId)
	{
		Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(*m_physicsId);
		Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(*m_physicsId);
		m_physicsId = nullptr;
	}

	if (!m_physicsSoftId)
	{
		auto mesh = dynamic_cast<Mesh*>(parent());


		m_softBodySharedSettings = new SoftBodySharedSettings;

		glm::mat4 normalMatrix = glm::transpose(glm::inverse(glm::mat4(mesh->parent()->finalMatrix())));

		if (m_settingsSoft.customVertices.size() > 0)
		{
			int i = 0;
			for (auto& vertex : mesh->verticesData().vertices)
			{
				SoftBodySharedSettings::Vertex v;

				vertex.position = mesh->parent()->finalMatrix() *
					glm::vec4(m_settingsSoft.customVertices[i].first, 1.0);
				vertex.normal = glm::normalize(normalMatrix * glm::vec4(vertex.normal, 1.0));
				v.mPosition = Float3(vertex.position.x, vertex.position.y, vertex.position.z);
				v.mInvMass = m_settingsSoft.customVertices[i].second;
				m_softBodySharedSettings->mVertices.push_back(v);
				i++;
			}
		}
		else
		{
			for (auto& vertex : mesh->verticesData().vertices)
			{
				SoftBodySharedSettings::Vertex v;

				vertex.position = mesh->parent()->finalMatrix() * glm::vec4(vertex.position, 1.0);
				vertex.normal = glm::normalize(normalMatrix * glm::vec4(vertex.normal, 1.0));
				// Normalize to unit vector
				v.mPosition = Float3(vertex.position.x, vertex.position.y, vertex.position.z);
				v.mInvMass = 1;
				m_softBodySharedSettings->mVertices.push_back(v);
			}
		}

		for (int i = 0; i < mesh->verticesData().indices.size(); i = i + 3)
		{
			m_softBodySharedSettings->AddFace(SoftBodySharedSettings::Face(mesh->verticesData().indices[i],
			                                                               mesh->verticesData().indices[i + 1],
			                                                               mesh->verticesData().indices[i + 2]));
		}

		m_softBodySharedSettings->CreateConstraints(&m_settingsSoft.vertexAttributes, 1);

		m_softBodySharedSettings->Optimize();


		SoftBodyCreationSettings sb_settings(m_softBodySharedSettings, Vec3::sZero(), Quat::sIdentity(),
		                                     m_collisionData.dynamic
			                                     ? Layers::MOVING
			                                     : Layers::NON_MOVING);
		sb_settings.mAllowSleeping = m_settingsSoft.sleep;
		sb_settings.mUpdatePosition = m_settingsSoft.updatePosition;
		sb_settings.mNumIterations = m_settingsSoft.numIteration;

		sb_settings.mFriction = m_collisionData.friction;
		sb_settings.mRestitution = m_collisionData.restitution;
		sb_settings.mPressure = m_collisionData.pressure;

		m_physicsSoftId = Physics::getInstance().bodyInterface().CreateSoftBody(sb_settings);
		Physics::getInstance().bodyInterface().AddBody(m_physicsSoftId->GetID(),
		                                               m_collisionData.dynamic
			                                               ? EActivation::Activate
			                                               : EActivation::DontActivate);
		mesh->parent()->matrix(glm::mat4(1.0));
		Prisma::MeshIndirect::getInstance().remove(0);
		Prisma::CacheScene::getInstance().updateSizes(true);
	}
}

Prisma::PhysicsMeshComponent::PhysicsMeshComponent() : Component{}
{
	name("Physics");
}
