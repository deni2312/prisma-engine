#include "../../include/Physics/Physics.h"
#include "../../include/Physics/PhysicsData.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include "../../include/Physics/DrawDebugger.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../../include/Components/PhysicsMeshComponent.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/SoftBody/SoftBodyMotionProperties.h"

struct PhysicsWorldJolt
{
	Prisma::BPLayerInterfaceImpl broad_phase_layer_interface;
	Prisma::ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	Prisma::ObjectLayerPairFilterImpl object_vs_object_layer_filter;
	PhysicsSystem physics_system;
	Prisma::MyBodyActivationListener body_activation_listener;
	Prisma::MyContactListener contact_listener;
	std::shared_ptr<TempAllocatorImpl> temp_allocator;
	std::shared_ptr<JobSystemThreadPool> job_system;
};

static std::shared_ptr<PhysicsWorldJolt> physicsWorldJolt = nullptr;

Prisma::Physics::Physics()
{
	physicsWorldJolt = std::make_shared<PhysicsWorldJolt>();
	RegisterDefaultAllocator();
	Factory::sInstance = new Factory();
	RegisterTypes();
	physicsWorldJolt->temp_allocator = std::make_shared<TempAllocatorImpl>(10 * 1024 * 1024);
	physicsWorldJolt->job_system = std::make_shared<JobSystemThreadPool>(
		cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
	constexpr uint cMaxBodies = 1024;
	constexpr uint cNumBodyMutexes = 0;
	constexpr uint cMaxBodyPairs = 1024;
	constexpr uint cMaxContactConstraints = 1024;
	physicsWorldJolt->physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
	                                      physicsWorldJolt->broad_phase_layer_interface,
	                                      physicsWorldJolt->object_vs_broadphase_layer_filter,
	                                      physicsWorldJolt->object_vs_object_layer_filter);
	physicsWorldJolt->physics_system.SetBodyActivationListener(&physicsWorldJolt->body_activation_listener);
	physicsWorldJolt->physics_system.SetContactListener(&physicsWorldJolt->contact_listener);
	physicsWorldJolt->physics_system.OptimizeBroadPhase();
	m_drawDebugger = new DrawDebugger();
}

void Prisma::Physics::update(float delta)
{
	physicsWorldJolt->physics_system.Update(delta, 1, &*physicsWorldJolt->temp_allocator,
	                                        &*physicsWorldJolt->job_system);

	m_indexVbo = 0;
	for (const auto& mesh : Prisma::GlobalData::getInstance().currentGlobalScene()->meshes)
	{
		auto physicsComponent = std::dynamic_pointer_cast<PhysicsMeshComponent>(
			mesh->components()["Physics"]);
		if (physicsComponent && physicsComponent->initPhysics())
		{
			if (physicsComponent->collisionData().softBody)
			{
				softBody(physicsComponent);
			}
			else
			{
				BodyLockRead lock(physicsSystem().GetBodyLockInterface(), physicsComponent->physicsId());
				if (lock.Succeeded())
				{
					auto& bInterface = lock.GetBody();
					const auto& matrix = mesh->parent()->matrix();
					auto id = physicsComponent->physicsId();
					glm::mat4 prismaMatrix(1.0);
					prismaMatrix = JfromMat4(bInterface.GetWorldTransform());
					auto scaledShape = static_cast<const ScaledShape*>(bInterface.GetShape());
					if (scaledShape)
					{
						prismaMatrix = scale(prismaMatrix, physicsComponent->scale());
						auto inverseMatrix = glm::inverse(mesh->parent()->parent()->finalMatrix());
						if (!mat4Equals(prismaMatrix, matrix))
						{
							mesh->parent()->matrix(inverseMatrix * prismaMatrix);
						}
					}
				}
			}
		}
		m_indexVbo = m_indexVbo + mesh->verticesData().vertices.size();
	}
}

BodyInterface& Prisma::Physics::bodyInterface()
{
	return physicsWorldJolt->physics_system.GetBodyInterface();
}

PhysicsSystem& Prisma::Physics::physicsSystem()
{
	return physicsWorldJolt->physics_system;
}

void Prisma::Physics::drawDebug()
{
	if (m_debug)
	{
		m_drawDebugger->line.setMVP(
			Prisma::GlobalData::getInstance().currentProjection() * Prisma::GlobalData::getInstance().
			                                                        currentGlobalScene()->camera->matrix());
		m_settings.mDrawShape = true;
		physicsWorldJolt->physics_system.DrawBodies(m_settings, m_drawDebugger);
		for (const auto& mesh : Prisma::GlobalData::getInstance().currentGlobalScene()->meshes)
		{
			auto physicsComponent = std::dynamic_pointer_cast<PhysicsMeshComponent>(mesh->components()["Physics"]);
			if (physicsComponent && physicsComponent->initPhysics())
			{
				if (physicsComponent->collisionData().softBody)
				{
					auto* softId = physicsComponent->softId();
					if (softId)
					{
						SoftBodyMotionProperties* mp = static_cast<SoftBodyMotionProperties*>(softId->
							GetMotionProperties());
						mp->DrawVertices(m_drawDebugger, JPH::Mat44::sIdentity());
					}
				}
			}
		}
	}
}

void Prisma::Physics::debug(bool debug)
{
	m_debug = debug;
}

bool Prisma::Physics::debug()
{
	if (m_debug)
	{
		m_drawDebugger->init();
	}
	return m_debug;
}

void Prisma::Physics::softBody(std::shared_ptr<Prisma::PhysicsMeshComponent> physics)
{
	auto mesh = dynamic_cast<Mesh*>(physics->parent());
	auto softId = physics->softId();
	if (softId)
	{
		BodyLockRead lock(physicsSystem().GetBodyLockInterface(), softId->GetID());
		if (lock.Succeeded())
		{
			SoftBodyMotionProperties* mp = static_cast<SoftBodyMotionProperties*>(softId->GetMotionProperties());
			auto& faces = mp->GetFaces();
			auto& verticesSoft = mp->GetVertices();

			auto& verticesData = mesh->verticesData();

			auto vao = Prisma::MeshIndirect::getInstance().vao();
			auto vbo = Prisma::MeshIndirect::getInstance().vbo();
			auto ebo = Prisma::MeshIndirect::getInstance().ebo();

			auto& verticesIndirect = Prisma::MeshIndirect::getInstance().verticesData();

			for (int i = 0; i < verticesData.vertices.size(); i++)
			{
				verticesData.vertices[i].position = Prisma::JfromVec3(verticesSoft[i].mPosition);
				verticesIndirect.vertices[m_indexVbo + i].position = verticesData.vertices[i].position;
			}

			vao->bind();

			vbo->writeSubData(verticesData.vertices.size() * sizeof(Prisma::Mesh::Vertex),
			                  m_indexVbo * sizeof(Prisma::Mesh::Vertex),
			                  verticesData.vertices.data());
			vao->resetVao();
		}
	}
}
