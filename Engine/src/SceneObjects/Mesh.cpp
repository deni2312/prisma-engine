#include "SceneObjects/Mesh.h"
#include "SceneObjects/Mesh.h"
#include "GlobalData/GlobalData.h"
#include "SceneData/MeshIndirect.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <string>

#include "../../../DiligentEngine/DiligentFX/Shaders/Common/public/ShaderDefinitions.fxh"
#include "GlobalData/CacheScene.h"
#include "Pipelines/PipelineHandler.h"

void Prisma::Mesh::loadModel(std::shared_ptr<VerticesData> vertices, bool compute)
{
	m_vertices = vertices;
	if (compute)
	{
		computeAABB();
	}
}

Prisma::Mesh::VerticesData& Prisma::Mesh::verticesData() const
{
	return *m_vertices;
}

void Prisma::Mesh::matrix(const glm::mat4& matrix, bool update)
{
	Node::matrix(matrix);
	CacheScene::getInstance().updateData(true);
}

glm::mat4 Prisma::Mesh::matrix() const
{
	return Node::matrix();
}

void Prisma::Mesh::finalMatrix(const glm::mat4& matrix, bool update)
{
	Node::finalMatrix(matrix);
	if (m_vectorId >= 0)
	{
		Prisma::MeshIndirect::getInstance().updateModels(m_vectorId);
	}
	CacheScene::getInstance().updateData(true);
}

glm::mat4 Prisma::Mesh::finalMatrix() const
{
	return Node::finalMatrix();
}

std::shared_ptr<Prisma::Mesh> Prisma::Mesh::instantiate(std::shared_ptr<Mesh> mesh)
{
	std::shared_ptr<Mesh> newInstance = nullptr;
	if (mesh)
	{
		newInstance = std::make_shared<Mesh>();
		newInstance->loadModel(std::make_shared<VerticesData>(*mesh->m_vertices));
		newInstance->material(std::make_shared<MaterialComponent>(*mesh->material()));
		newInstance->matrix(mesh->matrix());
		newInstance->name(mesh->name() + std::to_string(newInstance->uuid()));
		auto parent = std::make_shared<Node>();
		parent->name(mesh->parent()->name() + std::to_string(parent->uuid()));
		parent->matrix(mesh->parent()->matrix());
		Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
		parent->addChild(newInstance);
	}
	return newInstance;
}

void Prisma::Mesh::material(std::shared_ptr<MaterialComponent> material)
{
	m_material = material;
	addComponent(m_material);
	CacheScene::getInstance().updateTextures(true);
}

std::shared_ptr<Prisma::MaterialComponent> Prisma::Mesh::material()
{
	return m_material;
}

void Prisma::Mesh::vectorId(int vectorId)
{
	m_vectorId = vectorId;
}

int Prisma::Mesh::vectorId()
{
	return m_vectorId;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::Mesh::vBuffer()
{
	return m_vBuffer;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::Mesh::iBuffer()
{
	return m_iBuffer;
}

void createTLAS()
{
	
}

struct VertexBlas
{
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 uv;
};

void Prisma::Mesh::uploadBLAS()
{
	if (!m_blasGPU && m_vertices && !m_vertices->vertices.empty())
	{
		auto& contextData = Prisma::PrismaFunc::getInstance().contextData();


		std::vector<glm::vec3> vertices;

		std::vector<VertexBlas> verticesBlas;

		for (const auto& v : m_vertices->vertices)
		{
			vertices.push_back(v.position);

			verticesBlas.push_back({ v.position ,v.normal,v.texCoords });
		}

		Diligent::BufferDesc BuffDesc;
		BuffDesc.Name = "Cube";
		BuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
		BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffDesc.Size = sizeof(VertexBlas) * verticesBlas.size();

		Diligent::BufferData BufData = { verticesBlas.data(), BuffDesc.Size};

		contextData.m_pDevice->CreateBuffer(BuffDesc, &BufData, &m_CubeAttribsCB);
		VERIFY_EXPR(m_CubeAttribsCB != nullptr);

		//srb->GetVariableByName(Diligent::SHADER_TYPE_RAY_CLOSEST_HIT, "g_CubeAttribsCB")->Set(m_CubeAttribsCB);


		Diligent::BufferDesc VertBuffDesc;
		VertBuffDesc.Name = "Cube vertex buffer";
		VertBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
		VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_RAY_TRACING;
		VertBuffDesc.Size = sizeof(glm::vec3) * vertices.size();
		Diligent::BufferData VBData;
		VBData.pData = vertices.data();
		VBData.DataSize = sizeof(glm::vec3)* vertices.size();
		contextData.m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_vBuffer);

		Diligent::BufferDesc IndBuffDesc;
		IndBuffDesc.Name = "Cube index buffer";
		IndBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
		IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER | Diligent::BIND_RAY_TRACING;
		IndBuffDesc.Size = sizeof(unsigned int)* m_vertices->indices.size();
		Diligent::BufferData IBData;
		IBData.pData = m_vertices->indices.data();
		IBData.DataSize = sizeof(unsigned int) * m_vertices->indices.size();
		contextData.m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_iBuffer);

		// Create & build bottom level acceleration structure
		// Create BLAS
		Diligent::BLASTriangleDesc Triangles;
		{
			Triangles.GeometryName = "Cube Instance 1";
			Triangles.MaxVertexCount = vertices.size();
			Triangles.VertexValueType = Diligent::VT_FLOAT32;
			Triangles.VertexComponentCount = 3;
			Triangles.MaxPrimitiveCount = m_vertices->indices.size() / 3;
			Triangles.IndexType = Diligent::VT_UINT32;

			Diligent::BottomLevelASDesc ASDesc;
			ASDesc.Name = "Cube Instance 1";
			ASDesc.Flags = Diligent::RAYTRACING_BUILD_AS_PREFER_FAST_TRACE;
			ASDesc.pTriangles = &Triangles;
			ASDesc.TriangleCount = 1;

			contextData.m_pDevice->CreateBLAS(ASDesc, &m_pCubeBLAS);
			VERIFY_EXPR(m_pCubeBLAS != nullptr);
		}

		// Create scratch buffer
		Diligent::RefCntAutoPtr<Diligent::IBuffer> pScratchBuffer;
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "BLAS Scratch Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_RAY_TRACING;
			BuffDesc.Size = m_pCubeBLAS->GetScratchBufferSizes().Build;

			contextData.m_pDevice->CreateBuffer(BuffDesc, nullptr, &pScratchBuffer);
			VERIFY_EXPR(pScratchBuffer != nullptr);
		}

		// Build BLAS
		Diligent::BLASBuildTriangleData TriangleData;
		TriangleData.GeometryName = Triangles.GeometryName;
		TriangleData.pVertexBuffer = m_vBuffer;
		TriangleData.VertexStride = sizeof(glm::vec3);
		TriangleData.VertexCount = Triangles.MaxVertexCount;
		TriangleData.VertexValueType = Triangles.VertexValueType;
		TriangleData.VertexComponentCount = Triangles.VertexComponentCount;
		TriangleData.pIndexBuffer = m_iBuffer;
		TriangleData.PrimitiveCount = Triangles.MaxPrimitiveCount;
		TriangleData.IndexType = Triangles.IndexType;
		TriangleData.Flags = Diligent::RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		Diligent::BuildBLASAttribs Attribs;
		Attribs.pBLAS = m_pCubeBLAS;
		Attribs.pTriangleData = &TriangleData;
		Attribs.TriangleDataCount = 1;

		// Scratch buffer will be used to store temporary data during BLAS build.
		// Previous content in the scratch buffer will be discarded.
		Attribs.pScratchBuffer = pScratchBuffer;

		// Allow engine to change resource states.
		Attribs.BLASTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
		Attribs.GeometryTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
		Attribs.ScratchBufferTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

		contextData.m_pImmediateContext->BuildBLAS(Attribs);

		Prisma::PipelineHandler::getInstance().raytracing()->pso()->CreateShaderResourceBinding(&m_srb, true);
		m_blasGPU = true;
	}
}

Diligent::RefCntAutoPtr<Diligent::IBottomLevelAS> Prisma::Mesh::blas()
{
	return m_pCubeBLAS;
}

Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> Prisma::Mesh::raytracingSrb()
{
	return m_srb;
}

void Prisma::Mesh::computeAABB()
{
	auto vertices = verticesData().vertices;
	auto indices = verticesData().indices;
	if (vertices.empty())
	{
		m_aabbData = AABBData{glm::vec3(0.0f), glm::vec3(0.0f)};
	}

	glm::vec3 minPoint = vertices[0].position;
	glm::vec3 maxPoint = vertices[0].position;

	for (const auto& vertex : vertices)
	{
		minPoint.x = std::min(minPoint.x, vertex.position.x);
		minPoint.y = std::min(minPoint.y, vertex.position.y);
		minPoint.z = std::min(minPoint.z, vertex.position.z);

		maxPoint.x = std::max(maxPoint.x, vertex.position.x);
		maxPoint.y = std::max(maxPoint.y, vertex.position.y);
		maxPoint.z = std::max(maxPoint.z, vertex.position.z);
	}
	m_aabbData.min = minPoint;
	m_aabbData.max = maxPoint;

	m_aabbData.center = (m_aabbData.max + m_aabbData.min) * glm::vec3(0.5);
	m_aabbData.extents = m_aabbData.max - m_aabbData.center;
}

Prisma::Mesh::AABBData Prisma::Mesh::aabbData()
{
	return m_aabbData;
}
