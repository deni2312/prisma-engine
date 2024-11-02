#include "../../include/Components/PhysicsMeshComponent.h"
#include "bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "../../include/Physics/PhysicsData.h"
#include <glm/gtx/string_cast.hpp>

#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"

void Prisma::PhysicsMeshComponent::ui()
{
	ComponentType componentType;
	m_status.currentitem = static_cast<int>(m_collisionData.collider);
	m_status.items.push_back("BOX COLLIDER");
	m_status.items.push_back("SPHERE COLLIDER");
	m_status.items.push_back("LANDSCAPE COLLIDER");
	m_status.items.push_back("CONVEX COLLIDER");
	componentType = std::make_tuple(TYPES::STRINGLIST, "Collider", &m_status);

	ComponentType componentMass;
	componentMass = std::make_tuple(TYPES::FLOAT, "Mass", &m_collisionData.mass);

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

	ComponentType componentSoftBody;
	componentSoftBody = std::make_tuple(TYPES::BOOL, "Soft Body", &m_collisionData.softBody);

	componentButton = std::make_tuple(TYPES::BUTTON, "Apply Collider", &m_apply);

	addGlobal(componentType);

	addGlobal(componentMass);

	addGlobal(componentDynamic);

	addGlobal(componentButton);

	addGlobal(componentSoftBody);

	updateCollisionData();
}

void Prisma::PhysicsMeshComponent::update()
{
}

void Prisma::PhysicsMeshComponent::destroy()
{
	Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(m_physicsId);
	Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(m_physicsId);
	Component::destroy();
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
			if (!m_initPhysics)
			{
				m_physicsId = Physics::getInstance().bodyInterface().CreateAndAddBody(
					bodySettings, m_collisionData.dynamic ? EActivation::Activate : EActivation::DontActivate);
				m_initPhysics = true;
			}
			else
			{
				Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().RemoveBody(m_physicsId);
				Physics::getInstance().physicsSystem().GetBodyInterfaceNoLock().DestroyBody(m_physicsId);
				m_physicsId = Physics::getInstance().bodyInterface().CreateAndAddBody(
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
	if (!m_initPhysics)
	{
		updateCollisionData();
	}
}

BodyID& Prisma::PhysicsMeshComponent::physicsId()
{
	return m_physicsId;
}

bool Prisma::PhysicsMeshComponent::initPhysics()
{
	return m_initPhysics;
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
	decompose(mesh->parent()->matrix(), scale, rotation, translation, skew, perspective);

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
	aabbSettings.mUserData = uuid();
	return aabbSettings;
}

void Prisma::PhysicsMeshComponent::addSoftBody()
{
	auto mesh = dynamic_cast<Mesh*>(parent());
	m_softBodySharedSettings = new SoftBodySharedSettings;
	for (auto vertex : mesh->verticesData().vertices)
	{
		SoftBodySharedSettings::Vertex v;
		v.mPosition = Float3(v.mPosition.x, v.mPosition.y, v.mPosition.z);
		m_softBodySharedSettings->mVertices.push_back(v);
	}

	for (int i = 0; i < mesh->verticesData().indices.size() - 3; i = i + 3)
	{
		m_softBodySharedSettings->AddFace(SoftBodySharedSettings::Face(mesh->verticesData().indices[i],
		                                                               mesh->verticesData().indices[i + 1],
		                                                               mesh->verticesData().indices[i + 2]));
	}
}

Prisma::PhysicsMeshComponent::PhysicsMeshComponent() : Component{}
{
	name("Physics");
}
