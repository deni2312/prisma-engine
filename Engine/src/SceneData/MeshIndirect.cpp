#include "SceneData/MeshIndirect.h"
#include "GlobalData/GlobalData.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

#include "GlobalData/GlobalData.h"
#include <glm/gtx/string_cast.hpp>
#include "GlobalData/CacheScene.h"
#include "Helpers/SettingsLoader.h"
#include "Helpers/FrustumCulling.h"
#include "GlobalData/GlobalShaderNames.h"


void Prisma::MeshIndirect::sort() const {
        //auto& meshes = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes;
        //if (!meshes.empty())
        //{
        //	m_shaderCopy->use();
        //	m_shaderCopy->setInt(m_indicesCopyLocation, 0);
        //	m_shaderCopy->dispatchCompute({1, 1, 1});
        //	m_shaderCopy->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        //	auto camera = Prisma::GlobalData::getInstance().currentGlobalScene()->camera;
        //	Prisma::Settings globalSettings = Prisma::SettingsLoader::getInstance().getSettings();
        //	CameraData data;
        //	data.zFar = camera->farPlane();
        //	data.zNear = camera->nearPlane();
        //	data.fovY = glm::radians(camera->angle());
        //	data.aspect = static_cast<float>(globalSettings.width) / static_cast<float>(globalSettings.height);
        //	m_ssboCamera->modifyData(0, sizeof(CameraData), &data);
        //	m_shaderCopy->setInt(m_indicesCopyLocation, 1);
        //	m_shaderCopy->dispatchCompute({ meshes.size(), 1, 1 });
        //	m_shaderCopy->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        //	if (Prisma::GlobalData::getInstance().transparencies()) {
        //		m_shader->use();
        //		m_shader->dispatchCompute({ 1, 1, 1 });
        //		m_shader->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        //	}
        //	m_shaderCopy->use();
        //	m_shaderCopy->setInt(m_indicesCopyLocation, 2);
        //	m_shaderCopy->dispatchCompute({ meshes.size(), 1, 1 });
        //	m_shaderCopy->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        //}
}

void Prisma::MeshIndirect::updateStatusShader() const {
        //int size = Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size();
        //m_statusShader->use();
        //m_statusShader->setInt(m_sizeLocation, size);
        //m_statusShader->dispatchCompute({
        //	size +
        //	Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size(),
        //	1, 1
        //});
        //m_statusShader->wait(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void Prisma::MeshIndirect::updateIndirectBuffer() {
        m_drawCommands.clear();

        auto& contextData = PrismaFunc::getInstance().contextData();
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;

        m_currentIndex = 0;
        m_currentVertex = 0;
        for (const auto& mesh : meshes) {
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
        m_indirectBuffer.Release();

        Diligent::BufferDesc IndirectBufferDesc;
        IndirectBufferDesc.Name = "Indirect Draw Command Buffer";
        IndirectBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        IndirectBufferDesc.BindFlags = Diligent::BIND_INDIRECT_DRAW_ARGS;
        IndirectBufferDesc.Size = sizeof(DrawElementsIndirectCommand) * meshes.size();
        IndirectBufferDesc.ElementByteStride = sizeof(DrawElementsIndirectCommand);
        Diligent::BufferData InitData;
        InitData.pData = m_drawCommands.data();
        InitData.DataSize = sizeof(DrawElementsIndirectCommand) * meshes.size();
        contextData.m_pDevice->CreateBuffer(IndirectBufferDesc, &InitData, &m_indirectBuffer);

        m_commandsBuffer.DrawCount = meshes.size();
        m_commandsBuffer.DrawArgsOffset = 0;
        m_commandsBuffer.Flags = Diligent::DRAW_FLAGS::DRAW_FLAG_VERIFY_STATES;
        m_commandsBuffer.IndexType = Diligent::VT_UINT32;
        m_commandsBuffer.pAttribsBuffer = m_indirectBuffer;
}

void Prisma::MeshIndirect::updateIndirectBufferAnimation() {
        m_drawCommandsAnimation.clear();

        auto& contextData = PrismaFunc::getInstance().contextData();
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->animateMeshes;

        m_currentIndexAnimation = 0;
        m_currentVertexAnimation = 0;
        for (const auto& mesh : meshes) {
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
        m_indirectBufferAnimation.Release();

        Diligent::BufferDesc IndirectBufferDesc;
        IndirectBufferDesc.Name = "Animation Indirect Draw Command Buffer";
        IndirectBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        IndirectBufferDesc.BindFlags = Diligent::BIND_INDIRECT_DRAW_ARGS;
        IndirectBufferDesc.Size = sizeof(DrawElementsIndirectCommand) * meshes.size();
        IndirectBufferDesc.ElementByteStride = sizeof(DrawElementsIndirectCommand);
        Diligent::BufferData InitData;
        InitData.pData = m_drawCommandsAnimation.data();
        InitData.DataSize = sizeof(DrawElementsIndirectCommand) * meshes.size();
        contextData.m_pDevice->CreateBuffer(IndirectBufferDesc, &InitData, &m_indirectBufferAnimation);

        m_commandsBufferAnimation.DrawCount = meshes.size();
        m_commandsBufferAnimation.DrawArgsOffset = 0;
        m_commandsBufferAnimation.Flags = Diligent::DRAW_FLAGS::DRAW_FLAG_VERIFY_STATES;
        m_commandsBufferAnimation.IndexType = Diligent::VT_UINT32;
        m_commandsBufferAnimation.pAttribsBuffer = m_indirectBufferAnimation;
}

//std::shared_ptr<Prisma::VAO> Prisma::MeshIndirect::vao()
//{
//	return m_vao;
//}
//
//std::shared_ptr<Prisma::VBO> Prisma::MeshIndirect::vbo()
//{
//	return m_vbo;
//}
//
//std::shared_ptr<Prisma::EBO> Prisma::MeshIndirect::ebo()
//{
//	return m_ebo;
//}

void Prisma::MeshIndirect::updateTextureData() {
        m_textureViews.diffuse.clear();
        m_textureViews.normal.clear();
        m_textureViews.rm.clear();
        m_textureViews.specular.clear();

        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
        for (const auto& material : meshes) {
                m_textureViews.diffuse.push_back(
                        material->material()->diffuse()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                m_textureViews.normal.push_back(
                        material->material()->normal()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                m_textureViews.rm.push_back(
                        material->material()->roughnessMetalness()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                m_textureViews.specular.push_back(
                        material->material()->specular()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
        }
}

void Prisma::MeshIndirect::updateTextureDataAnimation() {
        m_textureViewsAnimation.diffuse.clear();
        m_textureViewsAnimation.normal.clear();
        m_textureViewsAnimation.rm.clear();
        m_textureViewsAnimation.specular.clear();

        auto& meshes = GlobalData::getInstance().currentGlobalScene()->animateMeshes;
        for (const auto& material : meshes) {
                m_textureViewsAnimation.diffuse.push_back(
                        material->material()->diffuse()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                m_textureViewsAnimation.normal.push_back(
                        material->material()->normal()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                m_textureViewsAnimation.rm.push_back(
                        material->material()->roughnessMetalness()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                m_textureViewsAnimation.specular.push_back(
                        material->material()->specular()[0].texture()->GetDefaultView(
                                Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
        }
}

void Prisma::MeshIndirect::updatePso() {
        for (auto resizeHandler : m_resizeHandler) {
                resizeHandler(m_modelBuffer, m_textureViews);
        }
}

Prisma::Mesh::VerticesData& Prisma::MeshIndirect::verticesData() {
        return m_verticesData;
}

void Prisma::MeshIndirect::load() {
        updateSize();
}

void Prisma::MeshIndirect::init() {
        m_cacheAdd.clear();
        m_cacheAddAnimate.clear();
        m_cacheRemove.clear();
        m_cacheRemoveAnimate.clear();

        for (int i = 0; i < GlobalData::getInstance().currentGlobalScene()->meshes.size(); i++) {
                getInstance().add(i);
        }

        for (int i = 0; i < GlobalData::getInstance().currentGlobalScene()->animateMeshes.size(); i++) {
                getInstance().addAnimate(i);
        }
        CacheScene::getInstance().updateSizes(true);
}

void Prisma::MeshIndirect::add(const unsigned int add) {
        m_cacheAdd.push_back(add);
}

void Prisma::MeshIndirect::remove(const unsigned int remove) {
        m_cacheRemove.push_back(remove);
}

void Prisma::MeshIndirect::updateModels(int model) {
        m_updateModels[model] = model;
}

void Prisma::MeshIndirect::addAnimate(const unsigned int add) {
        m_cacheAddAnimate.push_back(add);
}

void Prisma::MeshIndirect::removeAnimate(const unsigned int remove) {
        m_cacheRemoveAnimate.push_back(remove);
}

void Prisma::MeshIndirect::updateModelsAnimate(int model) {
        m_updateModelsAnimate[model] = model;
}

void Prisma::MeshIndirect::renderMeshes() const {
        if (!GlobalData::getInstance().currentGlobalScene()->meshes.empty()) {
                auto& contextData = PrismaFunc::getInstance().contextData();
                contextData.m_pImmediateContext->DrawIndexedIndirect(m_commandsBuffer);
        }
}

void Prisma::MeshIndirect::renderMeshesCopy() const {
        if (!GlobalData::getInstance().currentGlobalScene()->meshes.empty()) {
                //m_vao->bind();
                //glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawCopy);
                //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_indirectCopySSBOId, m_indirectDrawCopy);
                //glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr,
                //                            static_cast<GLuint>(Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.
                //	                            size()), 0);
                //glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        }
}

void Prisma::MeshIndirect::renderAnimateMeshes() const {
        if (!GlobalData::getInstance().currentGlobalScene()->animateMeshes.empty()) {
                auto& contextData = PrismaFunc::getInstance().contextData();
                contextData.m_pImmediateContext->DrawIndexedIndirect(m_commandsBufferAnimation);
        }
}

void Prisma::MeshIndirect::update() {
        if (CacheScene::getInstance().updateSizes()) {
                updateSize();
        }
        if (CacheScene::getInstance().updateData()) {
                updateModels();
                updateModelsAnimation();
        }
        if (CacheScene::getInstance().updateTextures()) {
                updateTextureSize();
        }
        if (CacheScene::getInstance().updateStatus()) {
                updateStatus();
                updateStatusAnimation();
        }
        sort();
}

void Prisma::MeshIndirect::updateSize() {
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;

        //CLEAR DATA
        m_materialData.clear();
        m_updateModels.clear();
        auto& contextData = PrismaFunc::getInstance().contextData();

        if (!meshes.empty()) {
                std::vector<Mesh::MeshData> models;
                for (int i = 0; i < meshes.size(); i++) {
                        models.push_back({meshes[i]->parent()->finalMatrix(),
                                          transpose(inverse(meshes[i]->parent()->finalMatrix()))});
                        meshes[i]->vectorId(i);
                }

                resizeModels(models);

                updateTextureData();

                updateStatus();

                if (!m_cacheRemove.empty()) {
                        m_verticesData.vertices.clear();
                        m_verticesData.indices.clear();
                        m_cacheAdd.clear();
                        for (int i = 0; i < GlobalData::getInstance().currentGlobalScene()->meshes.size(); i++) {
                                getInstance().add(i);
                        }
                        m_currentVertexMax = 0;
                }

                uint64_t sizeVbo = 0;
                uint64_t sizeEbo = 0;

                uint64_t vboCache = 0;
                uint64_t eboCache = 0;

                // Calculate the initial cache sizes
                for (int i = 0; i < meshes.size() - m_cacheAdd.size(); i++) {
                        vboCache += meshes[i]->verticesData().vertices.size();
                        eboCache += meshes[i]->verticesData().indices.size();
                }

                // Keep track of the current position in the cache
                uint64_t currentVboCache = vboCache;
                uint64_t currentEboCache = eboCache;
                // PUSH VERTICES
                for (unsigned int i : m_cacheAdd) {
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
                for (unsigned int i : m_cacheAdd) {
                        sizeEbo += meshes[i]->verticesData().indices.size();
                        m_verticesData.indices.insert(
                                m_verticesData.indices.begin() + currentEboCache,
                                meshes[i]->verticesData().indices.begin(),
                                meshes[i]->verticesData().indices.end()
                                );
                        // Update the current position in the EBO cache
                        currentEboCache += meshes[i]->verticesData().indices.size();
                }

                if (!m_cacheAdd.empty()) {
                        //GENERATE CACHE DATA 

                        if (currentVboCache > m_currentVertexMax || currentEboCache > m_currentIndexMax ||
                            m_currentVertexMax == 0) {
                                m_currentVertexMax = m_verticesData.vertices.size() + m_cacheSize;
                                m_currentIndexMax = m_verticesData.indices.size() + m_cacheSize;
                                m_verticesData.vertices.resize(m_currentVertexMax);
                                m_verticesData.indices.resize(m_currentIndexMax);

                                m_vBuffer.Release();
                                m_iBuffer.Release();

                                Diligent::BufferDesc VertBuffDesc;
                                VertBuffDesc.Name = "Vertices Data";
                                VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
                                VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
                                VertBuffDesc.Size = m_currentVertexMax * sizeof(Mesh::Vertex);
                                Diligent::BufferData VBData;
                                VBData.pData = m_verticesData.vertices.data();
                                VBData.DataSize = m_currentVertexMax * sizeof(Mesh::Vertex);
                                contextData.m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_vBuffer);

                                Diligent::BufferDesc IndBuffDesc;
                                IndBuffDesc.Name = "Index Data";
                                IndBuffDesc.Usage = Diligent::USAGE_DEFAULT;
                                IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
                                IndBuffDesc.Size = sizeof(unsigned int) * m_currentIndexMax;
                                Diligent::BufferData IBData;
                                IBData.pData = m_verticesData.indices.data();
                                IBData.DataSize = sizeof(unsigned int) * m_currentIndexMax;
                                contextData.m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_iBuffer);
                        } else {
                                contextData.m_pImmediateContext->UpdateBuffer(
                                        m_vBuffer, vboCache * sizeof(Mesh::Vertex), sizeVbo * sizeof(Mesh::Vertex),
                                        &m_verticesData.vertices[vboCache],
                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                                contextData.m_pImmediateContext->UpdateBuffer(
                                        m_iBuffer, eboCache * sizeof(unsigned int), sizeEbo * sizeof(unsigned int),
                                        &m_verticesData.indices[eboCache],
                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                        }
                }

                m_cacheAdd.clear();
                m_cacheRemove.clear();

                updateIndirectBuffer();

                updatePso();
        }
        updateAnimation();
}

void Prisma::MeshIndirect::updateModels() {
        auto& contextData = PrismaFunc::getInstance().contextData();

        for (const auto& model : m_updateModels) {
                auto finalMatrix = GlobalData::getInstance().currentGlobalScene()->meshes[model.first]->parent()->
                        finalMatrix();
                auto meshData = std::make_shared<Mesh::MeshData>();
                meshData->model = finalMatrix;
                meshData->normal = transpose(inverse(finalMatrix));
                contextData.m_pImmediateContext->UpdateBuffer(m_modelBuffer, sizeof(Mesh::MeshData) * model.first,
                                                              sizeof(Mesh::MeshData), meshData.get(),
                                                              Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        m_updateModels.clear();
}

void Prisma::MeshIndirect::updateModelsAnimation() {
        auto& contextData = PrismaFunc::getInstance().contextData();

        for (const auto& model : m_updateModelsAnimate) {
                auto finalMatrix = GlobalData::getInstance().currentGlobalScene()->animateMeshes[model.first]->parent()
                        ->
                        finalMatrix();
                auto meshData = std::make_shared<Mesh::MeshData>();
                meshData->model = finalMatrix;
                meshData->normal = transpose(inverse(finalMatrix));
                contextData.m_pImmediateContext->UpdateBuffer(m_modelBufferAnimation,
                                                              sizeof(Mesh::MeshData) * model.first,
                                                              sizeof(Mesh::MeshData), meshData.get(),
                                                              Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
        m_updateModelsAnimate.clear();
}

Prisma::MeshIndirect::MeshIndirect() {
        /*glGenBuffers(1, &m_indirectDraw);
        glGenBuffers(1, &m_indirectDrawCopy);
        glGenBuffers(1, &m_indirectDrawAnimation);

        m_vao = std::make_shared<VAO>();
        m_vbo = std::make_shared<VBO>();
        m_ebo = std::make_shared<EBO>();

        m_vaoAnimation = std::make_shared<VAO>();
        m_vboAnimation = std::make_shared<VBO>();
        m_eboAnimation = std::make_shared<EBO>();
        unsigned int numData = 0;
        glGenBuffers(1, &m_sizeAtomic);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_sizeAtomic);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, m_sizeAtomic);
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        m_ssboMaterial = std::make_shared<SSBO>(0);

        m_ssboModel = std::make_shared<SSBO>(1);
        m_ssboIndices = std::make_shared<SSBO>(23);

        m_ssboStatus = std::make_shared<SSBO>(24);
        m_ssboStatusCopy = std::make_shared<SSBO>(25);
        m_ssboStatusAnimation = std::make_shared<SSBO>(26);
        m_ssboAABB = std::make_shared<SSBO>(27);
        m_ssboId = std::make_shared<SSBO>(29);

        m_ssboModelAnimation = std::make_shared<SSBO>(6);
        m_ssboMaterialAnimation = std::make_shared<SSBO>(7);

        m_shader = std::make_shared<Shader>("../../../Engine/Shaders/TransparentPipeline/compute.glsl");
        m_shaderCopy = std::make_shared<Shader>("../../../Engine/Shaders/TransparentPipeline/computeCopy.glsl");
        m_statusShader = std::make_shared<Shader>("../../../Engine/Shaders/StatusPipeline/compute.glsl");
        m_statusShader->use();
        m_sizeLocation = m_statusShader->getUniformPosition("size");
        m_shaderCopy->use();
        m_indicesCopyLocation = m_shaderCopy->getUniformPosition("initIndices");

        m_ssboCamera = std::make_shared<Prisma::SSBO>(28);
        m_ssboCamera->resize(sizeof(CameraData));*/
        createMeshBuffer();
        createMeshAnimationBuffer();
}

void Prisma::MeshIndirect::createMeshBuffer() {
        auto& contextData = PrismaFunc::getInstance().contextData();

        Diligent::BufferDesc MatBufferDesc;

        MatBufferDesc.Name = "Mesh Transform Buffer";
        MatBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        MatBufferDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        MatBufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        MatBufferDesc.ElementByteStride = sizeof(glm::mat4);
        MatBufferDesc.Size = sizeof(glm::mat4); // Ensure enough space
        contextData.m_pDevice->CreateBuffer(MatBufferDesc, nullptr, &m_modelBuffer);

        Diligent::BufferDesc VertBuffDesc;
        VertBuffDesc.Name = "Vertices Data";
        VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
        VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
        VertBuffDesc.Size = 1;
        contextData.m_pDevice->CreateBuffer(VertBuffDesc, nullptr, &m_vBuffer);

        Diligent::BufferDesc IndBuffDesc;
        IndBuffDesc.Name = "Index Data";
        IndBuffDesc.Usage = Diligent::USAGE_DEFAULT;
        IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
        IndBuffDesc.Size = 1;
        contextData.m_pDevice->CreateBuffer(IndBuffDesc, nullptr, &m_iBuffer);

        Diligent::BufferDesc IndirectBufferDesc;
        IndirectBufferDesc.Name = "Indirect Draw Command Buffer";
        IndirectBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        IndirectBufferDesc.BindFlags = Diligent::BIND_INDIRECT_DRAW_ARGS;
        IndirectBufferDesc.Size = sizeof(DrawElementsIndirectCommand);
        IndirectBufferDesc.ElementByteStride = sizeof(DrawElementsIndirectCommand);
        contextData.m_pDevice->CreateBuffer(IndirectBufferDesc, nullptr, &m_indirectBuffer);
}

void Prisma::MeshIndirect::createMeshAnimationBuffer() {
        auto& contextData = PrismaFunc::getInstance().contextData();

        Diligent::BufferDesc MatBufferDescAnimation;

        MatBufferDescAnimation.Name = "Animation Mesh Transform Buffer";
        MatBufferDescAnimation.Usage = Diligent::USAGE_DEFAULT;
        MatBufferDescAnimation.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        MatBufferDescAnimation.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        MatBufferDescAnimation.ElementByteStride = sizeof(glm::mat4);
        MatBufferDescAnimation.Size = sizeof(glm::mat4); // Ensure enough space
        contextData.m_pDevice->CreateBuffer(MatBufferDescAnimation, nullptr, &m_modelBufferAnimation);

        Diligent::BufferDesc VertBuffDescAnimation;
        VertBuffDescAnimation.Name = "Animation Vertices Data";
        VertBuffDescAnimation.Usage = Diligent::USAGE_DEFAULT;
        VertBuffDescAnimation.BindFlags = Diligent::BIND_VERTEX_BUFFER;
        VertBuffDescAnimation.Size = 1;
        contextData.m_pDevice->CreateBuffer(VertBuffDescAnimation, nullptr, &m_vBufferAnimation);

        Diligent::BufferDesc IndBuffDescAnimation;
        IndBuffDescAnimation.Name = "Animation Index Data";
        IndBuffDescAnimation.Usage = Diligent::USAGE_DEFAULT;
        IndBuffDescAnimation.BindFlags = Diligent::BIND_INDEX_BUFFER;
        IndBuffDescAnimation.Size = 1;
        contextData.m_pDevice->CreateBuffer(IndBuffDescAnimation, nullptr, &m_iBufferAnimation);

        Diligent::BufferDesc IndirectBufferDescAnimation;
        IndirectBufferDescAnimation.Name = "Animation Indirect Draw Command Buffer";
        IndirectBufferDescAnimation.Usage = Diligent::USAGE_DEFAULT;
        IndirectBufferDescAnimation.BindFlags = Diligent::BIND_INDIRECT_DRAW_ARGS;
        IndirectBufferDescAnimation.Size = sizeof(DrawElementsIndirectCommand);
        IndirectBufferDescAnimation.ElementByteStride = sizeof(DrawElementsIndirectCommand);
        contextData.m_pDevice->CreateBuffer(IndirectBufferDescAnimation, nullptr, &m_indirectBufferAnimation);
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::MeshIndirect::modelBuffer() {
        return m_modelBuffer;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::MeshIndirect::modelBufferAnimation() {
        return m_modelBufferAnimation;
}

Prisma::MeshIndirect::MaterialView& Prisma::MeshIndirect::textureViews() {
        return m_textureViews;
}

Prisma::MeshIndirect::MaterialView& Prisma::MeshIndirect::textureViewsAnimation() {
        return m_textureViewsAnimation;
}

Diligent::DrawIndexedIndirectAttribs Prisma::MeshIndirect::commandsBuffer() {
        return m_commandsBuffer;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::MeshIndirect::statusBuffer() {
        return m_statusBuffer;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::MeshIndirect::statusBufferAnimation() {
        return m_statusBufferAnimation;
}

void Prisma::MeshIndirect::resizeModels(std::vector<Mesh::MeshData>& models) {
        m_modelBuffer.Release();
        auto& contextData = PrismaFunc::getInstance().contextData();
        Diligent::BufferDesc MatBufferDesc;

        MatBufferDesc.Name = "Mesh Transform Buffer";
        MatBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        MatBufferDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        MatBufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        MatBufferDesc.ElementByteStride = sizeof(Mesh::MeshData);
        auto size = sizeof(Mesh::MeshData) * models.size();
        MatBufferDesc.Size = size; // Ensure enough space
        Diligent::BufferData InitData;
        InitData.pData = models.data();
        InitData.DataSize = size;
        contextData.m_pDevice->CreateBuffer(MatBufferDesc, &InitData, &m_modelBuffer);

        updatePso();
}

void Prisma::MeshIndirect::resizeModelsAnimation(std::vector<Mesh::MeshData>& models) {
        m_modelBufferAnimation.Release();
        auto& contextData = PrismaFunc::getInstance().contextData();
        Diligent::BufferDesc MatBufferDesc;

        MatBufferDesc.Name = "Mesh Transform Buffer";
        MatBufferDesc.Usage = Diligent::USAGE_DEFAULT;
        MatBufferDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        MatBufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        MatBufferDesc.ElementByteStride = sizeof(Mesh::MeshData);
        auto size = sizeof(Mesh::MeshData) * models.size();
        MatBufferDesc.Size = size; // Ensure enough space
        Diligent::BufferData InitData;
        InitData.pData = models.data();
        InitData.DataSize = size;
        contextData.m_pDevice->CreateBuffer(MatBufferDesc, &InitData, &m_modelBufferAnimation);

        updatePso();
}

void Prisma::MeshIndirect::updateAnimation() {
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->animateMeshes;

        //CLEAR DATA
        m_materialDataAnimation.clear();
        m_updateModelsAnimate.clear();
        if (!meshes.empty()) {
                std::vector<Mesh::MeshData> models;
                for (int i = 0; i < meshes.size(); i++) {
                        models.push_back({meshes[i]->parent()->finalMatrix(),
                                          transpose(inverse(meshes[i]->parent()->finalMatrix()))});
                        meshes[i]->vectorId(i);
                }
                resizeModelsAnimation(models);

                updateTextureDataAnimation();

                updateStatusAnimation();

                if (!m_cacheRemoveAnimate.empty()) {
                        m_verticesDataAnimation.vertices.clear();
                        m_verticesDataAnimation.indices.clear();
                        m_cacheAddAnimate.clear();
                        for (int i = 0; i < GlobalData::getInstance().currentGlobalScene()->animateMeshes.size(); i++) {
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
                for (int i = 0; i < meshes.size() - m_cacheAddAnimate.size(); i++) {
                        vboCache += meshes[i]->animateVerticesData()->vertices.size();
                        eboCache += meshes[i]->animateVerticesData()->indices.size();
                }

                // Keep track of the current position in the cache
                uint64_t currentVboCache = vboCache;
                uint64_t currentEboCache = eboCache;

                // PUSH VERTICES
                for (unsigned int i : m_cacheAddAnimate) {
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
                for (unsigned int i : m_cacheAddAnimate) {
                        sizeEbo += meshes[i]->animateVerticesData()->indices.size();
                        m_verticesDataAnimation.indices.insert(
                                m_verticesDataAnimation.indices.begin() + currentEboCache,
                                meshes[i]->animateVerticesData()->indices.begin(),
                                meshes[i]->animateVerticesData()->indices.end()
                                );
                        // Update the current position in the EBO cache
                        currentEboCache += meshes[i]->animateVerticesData()->indices.size();
                }

                if (!m_cacheAddAnimate.empty()) {
                        //GENERATE CACHE DATA 
                        auto& contextData = PrismaFunc::getInstance().contextData();

                        if (currentVboCache > m_currentVertexMaxAnimation || m_cacheRemoveAnimate.size() > 0 ||
                            currentEboCache >
                            m_currentIndexMaxAnimation || m_currentVertexMaxAnimation == 0) {
                                m_currentVertexMaxAnimation = m_verticesDataAnimation.vertices.size() + m_cacheSize;
                                m_currentIndexMaxAnimation = m_verticesDataAnimation.indices.size() + m_cacheSize;
                                m_verticesDataAnimation.vertices.resize(m_currentVertexMaxAnimation);
                                m_verticesDataAnimation.indices.resize(m_currentIndexMaxAnimation);
                                m_vBufferAnimation.Release();
                                m_iBufferAnimation.Release();

                                Diligent::BufferDesc VertBuffDesc;
                                VertBuffDesc.Name = "Animation Vertices Data";
                                VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
                                VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
                                VertBuffDesc.Size = m_currentVertexMaxAnimation * sizeof(AnimatedMesh::AnimateVertex);
                                Diligent::BufferData VBData;
                                VBData.pData = m_verticesDataAnimation.vertices.data();
                                VBData.DataSize = m_currentVertexMaxAnimation * sizeof(AnimatedMesh::AnimateVertex);
                                contextData.m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_vBufferAnimation);

                                Diligent::BufferDesc IndBuffDesc;
                                IndBuffDesc.Name = "Animation Index Data";
                                IndBuffDesc.Usage = Diligent::USAGE_DEFAULT;
                                IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
                                IndBuffDesc.Size = m_currentIndexMaxAnimation * sizeof(unsigned int);
                                Diligent::BufferData IBData;
                                IBData.pData = m_verticesDataAnimation.indices.data();
                                IBData.DataSize = m_currentIndexMaxAnimation * sizeof(unsigned int);
                                contextData.m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_iBufferAnimation);

                                /*m_vaoAnimation->addAttribPointer(0, 3, sizeof(AnimatedMesh::AnimateVertex), nullptr);
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
                                                                 (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, m_Weights));*/
                        } else {
                                contextData.m_pImmediateContext->UpdateBuffer(
                                        m_vBufferAnimation, vboCache * sizeof(AnimatedMesh::AnimateVertex),
                                        sizeVbo * sizeof(AnimatedMesh::AnimateVertex),
                                        &m_verticesDataAnimation.vertices[vboCache],
                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                                contextData.m_pImmediateContext->UpdateBuffer(
                                        m_iBufferAnimation, eboCache * sizeof(unsigned int),
                                        sizeEbo * sizeof(unsigned int), &m_verticesDataAnimation.indices[eboCache],
                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                        }
                }

                m_cacheAddAnimate.clear();
                m_cacheRemoveAnimate.clear();

                updateIndirectBufferAnimation();
        }
}

void Prisma::MeshIndirect::updateTextureSize() {
        updateTextureData();
        updateTextureDataAnimation();
        updatePso();
        //m_ssboMaterial->resize(sizeof(MaterialData) * (m_materialData.size()));
        //m_ssboMaterial->modifyData(0, sizeof(MaterialData) * m_materialData.size(), m_materialData.data());

        //m_materialDataAnimation.clear();
        //auto& meshesAnimation = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes;
        //for (auto material : meshesAnimation)
        //{
        //	m_materialDataAnimation.push_back({
        //		material->material()->diffuse()[0].id(), material->material()->normal()[0].id(),
        //		material->material()->roughnessMetalness()[0].id(), material->material()->specular()[0].id(),
        //		material->material()->ambientOcclusion()[0].id(), material->material()->transparent(), 0.0,material->material()->color()
        //	});
        //}
        //m_ssboMaterialAnimation->resize(sizeof(MaterialData) * (m_materialDataAnimation.size()));
        //m_ssboMaterialAnimation->modifyData(0, sizeof(MaterialData) * m_materialDataAnimation.size(),
        //                                    m_materialDataAnimation.data());
}

void Prisma::MeshIndirect::updateStatus() {
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
        auto& contextData = PrismaFunc::getInstance().contextData();
        if (!meshes.empty()) {
                if (m_statusBuffer) {
                        m_statusBuffer.Release();
                }

                std::vector<StatusData> status;
                for (const auto& mesh : meshes) {
                        auto material = mesh->material();
                        auto rtMaterial = material->rtMaterial();
                        status.push_back({mesh->visible(), material->plain(), material->transparent(),
                                          material->isSpecular(), rtMaterial.GlassReflectionColorMask,
                                          rtMaterial.GlassAbsorption, rtMaterial.GlassMaterialColor,
                                          rtMaterial.GlassIndexOfRefraction, rtMaterial.GlassEnableDispersion,
                                          rtMaterial.DispersionSampleCount, material->metalness(),
                                          material->roughness()});
                }

                Diligent::BufferDesc statusDesc;
                statusDesc.Name = "Status Buffer";
                statusDesc.Usage = Diligent::USAGE_DEFAULT;
                statusDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
                statusDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
                statusDesc.ElementByteStride = sizeof(StatusData);
                statusDesc.Size = sizeof(StatusData) * status.size();
                Diligent::BufferData data;
                data.DataSize = statusDesc.Size;
                data.pData = status.data();
                contextData.m_pDevice->CreateBuffer(statusDesc, &data, &m_statusBuffer);
                updateIndirectBuffer();
                updatePso();
        }
        //auto animateMeshes = Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes;

        //if (!animateMeshes.empty())
        //{
        //	std::vector<StatusData> status;
        //	for (const auto& animateMesh : animateMeshes)
        //	{
        //		status.push_back({ animateMesh->visible(),animateMesh->material()->plain(),glm::vec2(0.0) });
        //	}
        //	m_ssboStatusAnimation->resize(sizeof(StatusData) * status.size());
        //	m_ssboStatusAnimation->modifyData(0, sizeof(StatusData) * status.size(), status.data());
        //	updateStatusShader();
        //}
}

void Prisma::MeshIndirect::updateStatusAnimation() {
        auto& meshes = GlobalData::getInstance().currentGlobalScene()->animateMeshes;
        auto& contextData = PrismaFunc::getInstance().contextData();
        if (!meshes.empty()) {
                if (m_statusBufferAnimation) {
                        m_statusBufferAnimation.Release();
                }

                std::vector<StatusData> status;
                for (const auto& mesh : meshes) {
                        auto material = mesh->material();
                        auto rtMaterial = material->rtMaterial();
                        status.push_back({mesh->visible(), material->plain(), material->transparent(),
                                          material->isSpecular(), rtMaterial.GlassReflectionColorMask,
                                          rtMaterial.GlassAbsorption, rtMaterial.GlassMaterialColor,
                                          rtMaterial.GlassIndexOfRefraction, rtMaterial.GlassEnableDispersion,
                                          rtMaterial.DispersionSampleCount, material->metalness(),
                                          material->roughness()});
                }

                Diligent::BufferDesc statusDesc;
                statusDesc.Name = "Animation Status Buffer";
                statusDesc.Usage = Diligent::USAGE_DEFAULT;
                statusDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
                statusDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
                statusDesc.ElementByteStride = sizeof(StatusData);
                statusDesc.Size = sizeof(StatusData) * status.size();
                Diligent::BufferData data;
                data.DataSize = statusDesc.Size;
                data.pData = status.data();
                contextData.m_pDevice->CreateBuffer(statusDesc, &data, &m_statusBufferAnimation);
                updateIndirectBufferAnimation();
                updatePso();
        }
}

void Prisma::MeshIndirect::setupBuffers() {
        auto& contextData = PrismaFunc::getInstance().contextData();
        constexpr Diligent::Uint64 offsets[] = {0};
        Diligent::IBuffer* pBuffs[] = {m_vBuffer};
        contextData.m_pImmediateContext->SetVertexBuffers(0, _countof(pBuffs), pBuffs, offsets,
                                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                                          Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.m_pImmediateContext->SetIndexBuffer(m_iBuffer, 0,
                                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Prisma::MeshIndirect::setupBuffersAnimation() {
        auto& contextData = PrismaFunc::getInstance().contextData();
        constexpr Diligent::Uint64 offsets[] = {0};
        Diligent::IBuffer* pBuffs[] = {m_vBufferAnimation};
        contextData.m_pImmediateContext->SetVertexBuffers(0, _countof(pBuffs), pBuffs, offsets,
                                                          Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                                          Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.m_pImmediateContext->SetIndexBuffer(m_iBufferAnimation, 0,
                                                        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Prisma::MeshIndirect::addResizeHandler(
        std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>, MaterialView&)> resizeHandler) {
        m_resizeHandler.push_back(resizeHandler);
}