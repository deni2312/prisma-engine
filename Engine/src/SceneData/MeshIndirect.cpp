#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/GlobalData/GlobalData.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

#include "../../include/GlobalData/GlobalData.h"
#include <glm/gtx/string_cast.hpp>
#include "../../include/GlobalData/CacheScene.h"


void Prisma::MeshIndirect::sort() const
{
	auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
	if (!meshes.empty())
	{
		m_shader->use();
		m_shader->setInt(m_sizeMeshesLocation, meshes.size());
		m_shader->dispatchCompute({1, 1, 1});
		m_shader->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		m_shaderCopy->use();
		m_shaderCopy->dispatchCompute({meshes.size(), 1, 1});
		m_shaderCopy->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

void Prisma::MeshIndirect::updateStatusShader() const
{
	int size = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size();
	m_statusShader->use();
	m_statusShader->setInt(m_sizeLocation, size);
	m_statusShader->dispatchCompute({
		size +
		Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size(),
		1, 1
	});
	m_statusShader->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

std::shared_ptr<Prisma::VAO> Prisma::MeshIndirect::vao()
{
	return m_vao;
}

std::shared_ptr<Prisma::VBO> Prisma::MeshIndirect::vbo()
{
	return m_vbo;
}

std::shared_ptr<Prisma::EBO> Prisma::MeshIndirect::ebo()
{
	return m_ebo;
}

Prisma::Mesh::VerticesData& Prisma::MeshIndirect::verticesData()
{
	return m_verticesData;
}

void Prisma::MeshIndirect::load()
{
	updateSize();
}

void Prisma::MeshIndirect::init()
{
	m_cacheAdd.clear();
	m_cacheAddAnimate.clear();
	m_cacheRemove.clear();
	m_cacheRemoveAnimate.clear();

	for (int i = 0; i < Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size(); i++)
	{
		getInstance().add(i);
	}

	for (int i = 0; i < Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size(); i++)
	{
		getInstance().addAnimate(i);
	}
	CacheScene::getInstance().updateSizes(true);
}

void Prisma::MeshIndirect::add(const unsigned int add)
{
	m_cacheAdd.push_back(add);
}

void Prisma::MeshIndirect::remove(const unsigned int remove)
{
	m_cacheRemove.push_back(remove);
}

void Prisma::MeshIndirect::updateModels(int model)
{
	m_updateModels.push_back(model);
}

void Prisma::MeshIndirect::addAnimate(const unsigned int add)
{
	m_cacheAddAnimate.push_back(add);
}

void Prisma::MeshIndirect::removeAnimate(const unsigned int remove)
{
	m_cacheRemoveAnimate.push_back(remove);
}

void Prisma::MeshIndirect::updateModelsAnimate(int model)
{
	m_updateModelsAnimate.push_back(model);
}

void Prisma::MeshIndirect::renderMeshes() const
{
	if (!Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.empty())
	{
		m_vao->bind();
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDraw);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectSSBOId, m_indirectDraw);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr,
		                            static_cast<GLuint>(Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.
			                            size()), 0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}
}

void Prisma::MeshIndirect::renderMeshesCopy() const
{
	if (!Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.empty())
	{
		m_vao->bind();
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawCopy);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectCopySSBOId, m_indirectDrawCopy);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr,
		                            static_cast<GLuint>(Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.
			                            size()), 0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}
}

void Prisma::MeshIndirect::renderAnimateMeshes() const
{
	if (!Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.empty())
	{
		m_vaoAnimation->bind();
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawAnimation);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectAnimationSSBOId, m_indirectDrawAnimation);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr,
		                            static_cast<GLuint>(Prisma::GlobalData::getInstance().currentGlobalScene()->
			                            animateMeshes.size()), 0);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	}
}

void Prisma::MeshIndirect::update()
{
	if (CacheScene::getInstance().updateSizes())
	{
		updateSize();
	}
	if (CacheScene::getInstance().updateData())
	{
		updateModels();
	}
	if (CacheScene::getInstance().updateTextures())
	{
		updateTextureSize();
	}
	if (CacheScene::getInstance().updateStatus())
	{
		updateStatus();
	}
	sort();
}

void Prisma::MeshIndirect::updateSize()
{
	auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;

	//CLEAR DATA
	m_materialData.clear();
	m_drawCommands.clear();
	m_updateModels.clear();

	if (!meshes.empty())
	{
		std::vector<glm::mat4> models;
		for (int i = 0; i < meshes.size(); i++)
		{
			models.push_back(meshes[i]->parent()->finalMatrix());
			meshes[i]->vectorId(i);
		}

		//PUSH MODEL MATRICES TO AN SSBO WITH ID 1
		m_ssboModel->resize(sizeof(glm::mat4) * (models.size()));
		m_ssboModelCopy->resize(sizeof(glm::mat4) * (models.size()));
		m_ssboModelCopy->modifyData(0, sizeof(glm::mat4) * models.size(), models.data());

		//PUSH MATERIAL TO AN SSBO WITH ID 0
		for (const auto& material : meshes)
		{
			m_materialData.push_back({
				material->material()->diffuse()[0].id(), material->material()->normal()[0].id(),
				material->material()->roughness_metalness()[0].id(), material->material()->specular()[0].id(),
				material->material()->ambientOcclusion()[0].id(), material->material()->transparent(), 0.0
			});
		}
		m_ssboMaterial->resize(sizeof(MaterialData) * (m_materialData.size()));
		m_ssboMaterialCopy->resize(sizeof(MaterialData) * (m_materialData.size()));
		m_ssboMaterialCopy->modifyData(0, sizeof(MaterialData) * m_materialData.size(), m_materialData.data());

		m_ssboIndices->resize(sizeof(glm::ivec4) * meshes.size());

		std::vector<unsigned int> status;
		for (const auto& mesh : meshes)
		{
			status.push_back(mesh->visible());
		}
		m_ssboStatusCopy->resize(sizeof(unsigned int) * status.size());
		m_ssboStatusCopy->modifyData(0, sizeof(unsigned int) * status.size(), status.data());
		m_ssboStatus->resize(sizeof(unsigned int) * status.size());

		//GENERATE DATA TO SEND INDIRECT
		m_vao->bind();

		if (!m_cacheRemove.empty())
		{
			m_verticesData.vertices.clear();
			m_verticesData.indices.clear();
			m_cacheAdd.clear();
			for (int i = 0; i < Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size(); i++)
			{
				getInstance().add(i);
			}
			m_currentVertexMax = 0;
		}

		uint64_t sizeVbo = 0;
		uint64_t sizeEbo = 0;

		uint64_t vboCache = 0;
		uint64_t eboCache = 0;

		// Calculate the initial cache sizes
		for (int i = 0; i < meshes.size() - m_cacheAdd.size(); i++)
		{
			vboCache += meshes[i]->verticesData().vertices.size();
			eboCache += meshes[i]->verticesData().indices.size();
		}

		// Keep track of the current position in the cache
		uint64_t currentVboCache = vboCache;
		uint64_t currentEboCache = eboCache;
		// PUSH VERTICES
		for (unsigned int i : m_cacheAdd)
		{
			sizeVbo += meshes[i]->verticesData().vertices.size();
			m_verticesData.vertices.insert(
				m_verticesData.vertices.begin() + currentVboCache,
				meshes[i]->verticesData().vertices.begin(),
				meshes[i]->verticesData().vertices.end()
			);
			// Update the current position in the VBO cache
			currentVboCache += meshes[i]->verticesData().vertices.size();
		}

		// PUSH INDICES
		for (unsigned int i : m_cacheAdd)
		{
			sizeEbo += meshes[i]->verticesData().indices.size();
			m_verticesData.indices.insert(
				m_verticesData.indices.begin() + currentEboCache,
				meshes[i]->verticesData().indices.begin(),
				meshes[i]->verticesData().indices.end()
			);
			// Update the current position in the EBO cache
			currentEboCache += meshes[i]->verticesData().indices.size();
		}

		if (!m_cacheAdd.empty())
		{
			//GENERATE CACHE DATA 

			if (currentVboCache > m_currentVertexMax || currentEboCache > m_currentIndexMax || m_currentVertexMax == 0)
			{
				m_currentVertexMax = m_verticesData.vertices.size() + m_cacheSize;
				m_currentIndexMax = m_verticesData.indices.size() + m_cacheSize;
				m_verticesData.vertices.resize(m_currentVertexMax);
				m_verticesData.indices.resize(m_currentIndexMax);

				m_vbo->writeData(m_currentVertexMax * sizeof(Mesh::Vertex), &m_verticesData.vertices[0],
				                 GL_DYNAMIC_DRAW);
				m_ebo->writeData(m_currentIndexMax * sizeof(unsigned int), &m_verticesData.indices[0], GL_DYNAMIC_DRAW);

				m_vao->addAttribPointer(0, 3, sizeof(Mesh::Vertex), nullptr);
				m_vao->addAttribPointer(1, 3, sizeof(Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, normal));
				m_vao->addAttribPointer(2, 2, sizeof(Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, texCoords));
				m_vao->addAttribPointer(3, 3, sizeof(Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, tangent));
				m_vao->addAttribPointer(4, 3, sizeof(Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, bitangent));
			}
			else
			{
				m_vbo->writeSubData(sizeVbo * sizeof(Mesh::Vertex), vboCache * sizeof(Mesh::Vertex),
				                    &m_verticesData.vertices[vboCache]);
				m_ebo->writeSubData(sizeEbo * sizeof(unsigned int), eboCache * sizeof(unsigned int),
				                    &m_verticesData.indices[eboCache]);
			}
		}

		m_cacheAdd.clear();
		m_cacheRemove.clear();

		//BIND INDIRECT DRAW BUFFER AND SET OFFSETS
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawCopy);

		m_currentIndex = 0;
		m_currentVertex = 0;
		for (const auto& mesh : meshes)
		{
			const auto& indices = mesh->verticesData().indices;
			const auto& vertices = mesh->verticesData().vertices;
			DrawElementsIndirectCommand command{};
			command.count = static_cast<GLuint>(indices.size());
			command.instanceCount = mesh->visible();
			command.firstIndex = m_currentIndex;
			command.baseVertex = m_currentVertex;
			command.baseInstance = 0;

			m_drawCommands.push_back(command);
			m_currentIndex = m_currentIndex + indices.size();
			m_currentVertex = m_currentVertex + vertices.size();
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_indirectDrawCopy);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectCopySSBOId, m_indirectDrawCopy);
		// Upload the draw commands to the buffer
		glBufferData(GL_DRAW_INDIRECT_BUFFER, m_drawCommands.size() * sizeof(DrawElementsIndirectCommand),
		             m_drawCommands.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDraw);
		glBindBuffer(GL_ARRAY_BUFFER, m_indirectDraw);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectSSBOId, m_indirectDraw);
		// Upload the draw commands to the buffer
		glBufferData(GL_DRAW_INDIRECT_BUFFER, m_drawCommands.size() * sizeof(DrawElementsIndirectCommand), nullptr,
		             GL_DYNAMIC_DRAW);
	}
	updateAnimation();
}

void Prisma::MeshIndirect::updateModels()
{
	for (const auto& model : m_updateModels)
	{
		auto finalMatrix = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes[model]->parent()->
			finalMatrix();
		m_ssboModelCopy->modifyData(sizeof(glm::mat4) * model, sizeof(glm::mat4),
		                            glm::value_ptr(finalMatrix));
	}

	for (const auto& model : m_updateModelsAnimate)
	{
		auto finalMatrix = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes[model]->parent()->
			finalMatrix();
		m_ssboModelAnimation->modifyData(sizeof(glm::mat4) * model, sizeof(glm::mat4),
		                                 glm::value_ptr(finalMatrix));
	}

	m_updateModelsAnimate.clear();
}

Prisma::MeshIndirect::MeshIndirect()
{
	glGenBuffers(1, &m_indirectDraw);
	glGenBuffers(1, &m_indirectDrawCopy);
	glGenBuffers(1, &m_indirectDrawAnimation);

	m_vao = std::make_shared<VAO>();
	m_vbo = std::make_shared<VBO>();
	m_ebo = std::make_shared<EBO>();

	m_vaoAnimation = std::make_shared<VAO>();
	m_vboAnimation = std::make_shared<VBO>();
	m_eboAnimation = std::make_shared<EBO>();

	m_ssboModel = std::make_shared<SSBO>(1);
	m_ssboMaterial = std::make_shared<SSBO>(0);

	m_ssboModelCopy = std::make_shared<SSBO>(20);
	m_ssboMaterialCopy = std::make_shared<SSBO>(21);
	m_ssboIndices = std::make_shared<SSBO>(23);

	m_ssboStatus = std::make_shared<SSBO>(24);
	m_ssboStatusCopy = std::make_shared<SSBO>(25);
	m_ssboStatusAnimation = std::make_shared<SSBO>(26);

	m_ssboModelAnimation = std::make_shared<SSBO>(6);
	m_ssboMaterialAnimation = std::make_shared<SSBO>(7);

	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/TransparentPipeline/compute.glsl");
	m_shaderCopy = std::make_shared<Shader>("../../../Engine/Shaders/TransparentPipeline/computeCopy.glsl");
	m_statusShader = std::make_shared<Shader>("../../../Engine/Shaders/StatusPipeline/compute.glsl");
	m_statusShader->use();
	m_sizeLocation = m_statusShader->getUniformPosition("size");
	m_shader->use();
	m_sizeMeshesLocation = m_statusShader->getUniformPosition("size");
}

void Prisma::MeshIndirect::updateAnimation()
{
	auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes;

	//CLEAR DATA
	m_materialDataAnimation.clear();
	m_drawCommandsAnimation.clear();
	m_updateModelsAnimate.clear();
	if (!meshes.empty())
	{
		std::vector<glm::mat4> models;
		for (int i = 0; i < meshes.size(); i++)
		{
			models.push_back(meshes[i]->parent()->finalMatrix());
			meshes[i]->vectorId(i);
		}

		//PUSH MODEL MATRICES TO AN SSBO WITH ID 1
		m_ssboModelAnimation->resize(sizeof(glm::mat4) * (models.size()));
		m_ssboModelAnimation->modifyData(0, sizeof(glm::mat4) * models.size(), models.data());

		//PUSH MATERIAL TO AN SSBO WITH ID 0
		for (auto material : meshes)
		{
			m_materialDataAnimation.push_back({
				material->material()->diffuse()[0].id(), material->material()->normal()[0].id(),
				material->material()->roughness_metalness()[0].id(), material->material()->specular()[0].id(),
				material->material()->ambientOcclusion()[0].id(), material->material()->transparent(), 0.0
			});
		}
		m_ssboMaterialAnimation->resize(sizeof(MaterialData) * (m_materialDataAnimation.size()));
		m_ssboMaterialAnimation->modifyData(0, sizeof(MaterialData) * m_materialDataAnimation.size(),
		                                    m_materialDataAnimation.data());

		std::vector<unsigned int> status;
		for (const auto& mesh : meshes)
		{
			status.push_back(mesh->visible());
		}
		m_ssboStatusAnimation->resize(sizeof(unsigned int) * status.size());
		m_ssboStatusAnimation->modifyData(0, sizeof(unsigned int) * status.size(), status.data());

		//GENERATE DATA TO SEND INDIRECT
		m_vaoAnimation->bind();

		if (!m_cacheRemoveAnimate.empty())
		{
			m_verticesDataAnimation.vertices.clear();
			m_verticesDataAnimation.indices.clear();
			m_cacheAddAnimate.clear();
			for (int i = 0; i < Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size(); i++)
			{
				getInstance().addAnimate(i);
			}
			m_currentVertexAnimation = 0;
		}

		//GENERATE CACHE DATA

		uint64_t sizeVbo = 0;
		uint64_t sizeEbo = 0;

		uint64_t vboCache = 0;
		uint64_t eboCache = 0;

		// Calculate the initial cache sizes
		for (int i = 0; i < meshes.size() - m_cacheAddAnimate.size(); i++)
		{
			vboCache += meshes[i]->animateVerticesData()->vertices.size();
			eboCache += meshes[i]->animateVerticesData()->indices.size();
		}

		// Keep track of the current position in the cache
		uint64_t currentVboCache = vboCache;
		uint64_t currentEboCache = eboCache;

		// PUSH VERTICES
		for (unsigned int i : m_cacheAddAnimate)
		{
			sizeVbo += meshes[i]->animateVerticesData()->vertices.size();
			m_verticesDataAnimation.vertices.insert(
				m_verticesDataAnimation.vertices.begin() + currentVboCache,
				meshes[i]->animateVerticesData()->vertices.begin(),
				meshes[i]->animateVerticesData()->vertices.end()
			);
			// Update the current position in the VBO cache
			currentVboCache += meshes[i]->animateVerticesData()->vertices.size();
		}

		// PUSH INDICES
		for (unsigned int i : m_cacheAddAnimate)
		{
			sizeEbo += meshes[i]->animateVerticesData()->indices.size();
			m_verticesDataAnimation.indices.insert(
				m_verticesDataAnimation.indices.begin() + currentEboCache,
				meshes[i]->animateVerticesData()->indices.begin(),
				meshes[i]->animateVerticesData()->indices.end()
			);
			// Update the current position in the EBO cache
			currentEboCache += meshes[i]->animateVerticesData()->indices.size();
		}

		if (!m_cacheAddAnimate.empty())
		{
			//GENERATE CACHE DATA 

			if (currentVboCache > m_currentVertexMaxAnimation || m_cacheRemoveAnimate.size() > 0 || currentEboCache >
				m_currentIndexMaxAnimation || m_currentVertexMaxAnimation == 0)
			{
				m_currentVertexMaxAnimation = m_verticesDataAnimation.vertices.size() + m_cacheSize;
				m_currentIndexMaxAnimation = m_verticesDataAnimation.indices.size() + m_cacheSize;
				m_verticesDataAnimation.vertices.resize(m_currentVertexMaxAnimation);
				m_verticesDataAnimation.indices.resize(m_currentIndexMaxAnimation);
				m_vboAnimation->writeData(m_currentVertexMaxAnimation * sizeof(AnimatedMesh::AnimateVertex),
				                          &m_verticesDataAnimation.vertices[0], GL_DYNAMIC_DRAW);
				m_eboAnimation->writeData(m_currentIndexMaxAnimation * sizeof(unsigned int),
				                          &m_verticesDataAnimation.indices[0], GL_DYNAMIC_DRAW);

				m_vaoAnimation->addAttribPointer(0, 3, sizeof(AnimatedMesh::AnimateVertex), nullptr);
				m_vaoAnimation->addAttribPointer(1, 3, sizeof(AnimatedMesh::AnimateVertex),
				                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, normal));
				m_vaoAnimation->addAttribPointer(2, 2, sizeof(AnimatedMesh::AnimateVertex),
				                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, texCoords));
				m_vaoAnimation->addAttribPointer(3, 3, sizeof(AnimatedMesh::AnimateVertex),
				                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, tangent));
				m_vaoAnimation->addAttribPointer(4, 3, sizeof(AnimatedMesh::AnimateVertex),
				                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, bitangent));
				m_vaoAnimation->addAttribPointer(5, 4, sizeof(AnimatedMesh::AnimateVertex),
				                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, m_BoneIDs),
				                                 GL_INT);
				m_vaoAnimation->addAttribPointer(6, 4, sizeof(AnimatedMesh::AnimateVertex),
				                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, m_Weights));
			}
			else
			{
				m_vboAnimation->writeSubData(sizeVbo * sizeof(AnimatedMesh::AnimateVertex),
				                             vboCache * sizeof(AnimatedMesh::AnimateVertex),
				                             &m_verticesDataAnimation.vertices[vboCache]);
				m_eboAnimation->writeSubData(sizeEbo * sizeof(unsigned int), eboCache * sizeof(unsigned int),
				                             &m_verticesDataAnimation.indices[eboCache]);
			}
		}


		m_cacheAddAnimate.clear();
		m_cacheRemoveAnimate.clear();

		//BIND INDIRECT DRAW BUFFER AND SET OFFSETS
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawAnimation);

		m_currentIndexAnimation = 0;
		m_currentVertexAnimation = 0;
		for (const auto& mesh : meshes)
		{
			const auto& indices = mesh->animateVerticesData()->indices;
			const auto& vertices = mesh->animateVerticesData()->vertices;
			DrawElementsIndirectCommand command{};
			command.count = static_cast<GLuint>(indices.size());
			command.instanceCount = mesh->visible();
			command.firstIndex = m_currentIndexAnimation;
			command.baseVertex = m_currentVertexAnimation;
			command.baseInstance = 0;

			m_drawCommandsAnimation.push_back(command);
			m_currentIndexAnimation = m_currentIndexAnimation + indices.size();
			m_currentVertexAnimation = m_currentVertexAnimation + vertices.size();
		}
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectAnimationSSBOId, m_indirectDrawAnimation);
		// Upload the draw commands to the buffer
		glBufferData(GL_DRAW_INDIRECT_BUFFER, m_drawCommandsAnimation.size() * sizeof(DrawElementsIndirectCommand),
		             m_drawCommandsAnimation.data(), GL_DYNAMIC_DRAW);
	}
}

void Prisma::MeshIndirect::updateTextureSize()
{
	m_materialData.clear();
	auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
	for (auto material : meshes)
	{
		m_materialData.push_back({
			material->material()->diffuse()[0].id(), material->material()->normal()[0].id(),
			material->material()->roughness_metalness()[0].id(), material->material()->specular()[0].id(),
			material->material()->ambientOcclusion()[0].id(), material->material()->transparent(), 0.0
		});
	}
	m_ssboMaterialCopy->resize(sizeof(MaterialData) * (m_materialData.size()));
	m_ssboMaterial->resize(sizeof(MaterialData) * (m_materialData.size()));
	m_ssboMaterialCopy->modifyData(0, sizeof(MaterialData) * m_materialData.size(), m_materialData.data());

	m_materialDataAnimation.clear();
	auto& meshesAnimation = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes;
	for (auto material : meshesAnimation)
	{
		m_materialDataAnimation.push_back({
			material->material()->diffuse()[0].id(), material->material()->normal()[0].id(),
			material->material()->roughness_metalness()[0].id(), material->material()->specular()[0].id(),
			material->material()->ambientOcclusion()[0].id(), material->material()->transparent(), 0.0
		});
	}
	m_ssboMaterialAnimation->resize(sizeof(MaterialData) * (m_materialDataAnimation.size()));
	m_ssboMaterialAnimation->modifyData(0, sizeof(MaterialData) * m_materialDataAnimation.size(),
	                                    m_materialDataAnimation.data());
}

void Prisma::MeshIndirect::updateStatus() const
{
	auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
	if (!meshes.empty())
	{
		std::vector<unsigned int> status;
		for (const auto& mesh : meshes)
		{
			status.push_back(mesh->visible());
		}
		m_ssboStatusCopy->resize(sizeof(unsigned int) * status.size());
		m_ssboStatusCopy->modifyData(0, sizeof(unsigned int) * status.size(), status.data());
		m_ssboStatus->resize(sizeof(unsigned int) * status.size());
		updateStatusShader();
	}
	auto animateMeshes = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes;


	if (!animateMeshes.empty())
	{
		std::vector<unsigned int> status;
		for (const auto& animateMesh : animateMeshes)
		{
			status.push_back(animateMesh->visible());
		}
		m_ssboStatusAnimation->resize(sizeof(unsigned int) * status.size());
		m_ssboStatusAnimation->modifyData(0, sizeof(unsigned int) * status.size(), status.data());
		updateStatusShader();
	}
}
