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

std::shared_ptr<Prisma::MeshIndirect> Prisma::MeshIndirect::instance = nullptr;

void Prisma::MeshIndirect::load()
{
    updateSize();
}

void Prisma::MeshIndirect::renderMeshes()
{
    if (currentGlobalScene->meshes.size() > 0) {
        m_vao->bind();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDraw);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLuint>(currentGlobalScene->meshes.size()), 0);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }
}

void Prisma::MeshIndirect::renderAnimateMeshes()
{
    if (currentGlobalScene->animateMeshes.size() > 0) {
        m_vaoAnimation->bind();
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDrawAnimation);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, static_cast<GLuint>(currentGlobalScene->animateMeshes.size()), 0);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }
}

void Prisma::MeshIndirect::update()
{
    if (updateSizes) {
        updateSize();
    }
    if (updateData) {
        updateModels();
    }
    if (updateTextures) {
        updateTextureSize();
    }
}

void Prisma::MeshIndirect::updateSize()
{
    auto meshes = currentGlobalScene->meshes;

    if (meshes.size() > 0) {

        //CLEAR DATA
        m_verticesData.vertices.clear();
        m_verticesData.indices.clear();
        m_materialData.clear();
        m_drawCommands.clear();


        //PUSH VERTICES
        for (const auto& vertices : meshes) {
            m_verticesData.vertices.insert(m_verticesData.vertices.end(), vertices->verticesData().vertices.begin(), vertices->verticesData().vertices.end());
        }
        //PUSH INDICES
        for (const auto& indices : meshes) {
            m_verticesData.indices.insert(m_verticesData.indices.end(), indices->verticesData().indices.begin(), indices->verticesData().indices.end());
        }
        std::vector<glm::mat4> models;
        for (int i = 0; i < meshes.size(); i++) {
            models.push_back(meshes[i]->parent()->finalMatrix());
            meshes[i]->vectorId(i);
        }

        //PUSH MODEL MATRICES TO AN SSBO WITH ID 1
        m_ssboModel->resize(sizeof(glm::mat4) * (models.size()));
        m_ssboModel->modifyData(0, sizeof(glm::mat4) * models.size(), models.data());

        //PUSH MATERIAL TO AN SSBO WITH ID 0
        for (const auto& material : meshes) {
            m_materialData.push_back({ material->material()->diffuse()[0].id(),material->material()->normal()[0].id() ,material->material()->roughness_metalness()[0].id() ,glm::vec2(0.0f) });
        }
        m_ssboMaterial->resize(sizeof(Prisma::MaterialData) * (m_materialData.size()));
        m_ssboMaterial->modifyData(0, sizeof(Prisma::MaterialData) * m_materialData.size(), m_materialData.data());


        //GENERATE DATA TO SEND INDIRECT
        m_vao->bind();

        //GENERATE CACHE DATA
        m_currentVertexSize = m_verticesData.vertices.size();
        m_currentIndexSize = m_verticesData.indices.size();
        m_currentVertexMax = m_verticesData.vertices.size();
        m_currentIndexMax = m_verticesData.indices.size();

        m_vbo->writeData(m_currentVertexMax * sizeof(Prisma::Mesh::Vertex), &m_verticesData.vertices[0], GL_DYNAMIC_DRAW);

        m_ebo->writeData(m_currentIndexMax * sizeof(unsigned int), &m_verticesData.indices[0], GL_DYNAMIC_DRAW);

        m_vao->addAttribPointer(0, 3, sizeof(Prisma::Mesh::Vertex), (void*)0);
        m_vao->addAttribPointer(1, 3, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, normal));
        m_vao->addAttribPointer(2, 2, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, texCoords));
        m_vao->addAttribPointer(3, 3, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, tangent));
        m_vao->addAttribPointer(4, 3, sizeof(Prisma::Mesh::Vertex), (void*)offsetof(Prisma::Mesh::Vertex, bitangent));


        //BIND INDIRECT DRAW BUFFER AND SET OFFSETS
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectDraw);

        m_currentIndex = 0;
        m_currentVertex = 0;
        for (const auto& mesh : meshes)
        {
            const auto& indices = mesh->verticesData().indices;
            const auto& vertices = mesh->verticesData().vertices;
            DrawElementsIndirectCommand command{};
            command.count = static_cast<GLuint>(indices.size());
            command.instanceCount = 1;
            command.firstIndex = m_currentIndex;
            command.baseVertex = m_currentVertex;
            command.baseInstance = 0;

            m_drawCommands.push_back(command);
            m_currentIndex = m_currentIndex + indices.size();
            m_currentVertex = m_currentVertex + vertices.size();
        }
        // Upload the draw commands to the buffer
        glBufferData(GL_DRAW_INDIRECT_BUFFER, m_drawCommands.size() * sizeof(DrawElementsIndirectCommand), m_drawCommands.data(), GL_DYNAMIC_DRAW);
    }
    updateAnimation();

}

void Prisma::MeshIndirect::updateModels()
{
    std::vector<glm::mat4> models;
    for (const auto& model : currentGlobalScene->meshes) {
        models.push_back(model->parent()->finalMatrix());
    }
    m_ssboModel->modifyData(0, sizeof(glm::mat4) * models.size(), models.data());

    std::vector<glm::mat4> modelsAnimation;
    for (const auto& model : currentGlobalScene->animateMeshes) {
        modelsAnimation.push_back(model->parent()->finalMatrix());
    }
    m_ssboModelAnimation->modifyData(0, sizeof(glm::mat4) * modelsAnimation.size(), modelsAnimation.data());
}

Prisma::MeshIndirect::MeshIndirect()
{
    glGenBuffers(1, &m_indirectDraw);
    glGenBuffers(1, &m_indirectDrawAnimation);

    m_vao=std::make_shared<Prisma::VAO>();
    m_vbo=std::make_shared<Prisma::VBO>();
    m_ebo=std::make_shared<Prisma::EBO>();

    m_vaoAnimation = std::make_shared<Prisma::VAO>();
    m_vboAnimation = std::make_shared<Prisma::VBO>();
    m_eboAnimation = std::make_shared<Prisma::EBO>();

    m_ssboModel = std::make_shared<Prisma::SSBO>(1);
    m_ssboMaterial = std::make_shared<Prisma::SSBO>(0);

    m_ssboModelAnimation = std::make_shared<Prisma::SSBO>(6);
    m_ssboMaterialAnimation = std::make_shared<Prisma::SSBO>(7);
}

void Prisma::MeshIndirect::updateAnimation()
{
    auto meshes = currentGlobalScene->animateMeshes;

    if (meshes.size() > 0) {
        //CLEAR DATA
        m_verticesDataAnimation.vertices.clear();
        m_verticesDataAnimation.indices.clear();
        m_materialDataAnimation.clear();
        m_drawCommandsAnimation.clear();


        //PUSH VERTICES
        for (const auto& vertices : meshes) {
            m_verticesDataAnimation.vertices.insert(m_verticesDataAnimation.vertices.end(), vertices->animateVerticesData()->vertices.begin(), vertices->animateVerticesData()->vertices.end());
        }
        //PUSH INDICES
        for (const auto& indices : meshes) {
            m_verticesDataAnimation.indices.insert(m_verticesDataAnimation.indices.end(), indices->animateVerticesData()->indices.begin(), indices->animateVerticesData()->indices.end());
        }
        std::vector<glm::mat4> models;
        for (int i = 0; i < meshes.size(); i++) {
            models.push_back(meshes[i]->parent()->finalMatrix());
            meshes[i]->vectorId(i);
        }

        //PUSH MODEL MATRICES TO AN SSBO WITH ID 1
        m_ssboModelAnimation->resize(sizeof(glm::mat4) * (models.size()));
        m_ssboModelAnimation->modifyData(0, sizeof(glm::mat4) * models.size(), models.data());

        //PUSH MATERIAL TO AN SSBO WITH ID 0
        for (auto material : meshes) {
            m_materialDataAnimation.push_back({ material->material()->diffuse()[0].id(),material->material()->normal()[0].id() ,material->material()->roughness_metalness()[0].id() ,glm::vec2(0.0f) });
        }
        m_ssboMaterialAnimation->resize(sizeof(Prisma::MaterialData) * (m_materialDataAnimation.size()));
        m_ssboMaterialAnimation->modifyData(0, sizeof(Prisma::MaterialData) * m_materialDataAnimation.size(), m_materialDataAnimation.data());


        //GENERATE DATA TO SEND INDIRECT
        m_vaoAnimation->bind();

        //GENERATE CACHE DATA
        m_currentVertexSize = m_verticesDataAnimation.vertices.size();
        m_currentIndexSize = m_verticesDataAnimation.indices.size();
        m_currentVertexMax = m_verticesDataAnimation.vertices.size();
        m_currentIndexMax = m_verticesDataAnimation.indices.size();

        

        m_vboAnimation->writeData(m_currentVertexMax * sizeof(Prisma::AnimatedMesh::AnimateVertex), &m_verticesDataAnimation.vertices[0], GL_DYNAMIC_DRAW);

        m_eboAnimation->writeData(m_currentIndexMax * sizeof(unsigned int), &m_verticesDataAnimation.indices[0], GL_DYNAMIC_DRAW);

        m_vaoAnimation->addAttribPointer(0, 3, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)0);
        m_vaoAnimation->addAttribPointer(1, 3, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, normal));
        m_vaoAnimation->addAttribPointer(2, 2, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, texCoords));
        m_vaoAnimation->addAttribPointer(3, 3, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, tangent));
        m_vaoAnimation->addAttribPointer(4, 3, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, bitangent));
        m_vaoAnimation->addAttribPointer(5, 4, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, m_BoneIDs), GL_INT);
        m_vaoAnimation->addAttribPointer(6, 4, sizeof(Prisma::AnimatedMesh::AnimateVertex), (void*)offsetof(Prisma::AnimatedMesh::AnimateVertex, m_Weights));
        glBindVertexArray(0);
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
            command.instanceCount = 1;
            command.firstIndex = m_currentIndexAnimation;
            command.baseVertex = m_currentVertexAnimation;
            command.baseInstance = 0;

            m_drawCommandsAnimation.push_back(command);
            m_currentIndexAnimation = m_currentIndexAnimation + indices.size();
            m_currentVertexAnimation = m_currentVertexAnimation + vertices.size();
        }
        // Upload the draw commands to the buffer
        glBufferData(GL_DRAW_INDIRECT_BUFFER, m_drawCommandsAnimation.size() * sizeof(DrawElementsIndirectCommand), m_drawCommandsAnimation.data(), GL_DYNAMIC_DRAW);
    }
}

Prisma::MeshIndirect& Prisma::MeshIndirect::getInstance()
{
    if (!instance) {
        instance = std::make_shared<MeshIndirect>();
    }
    return *instance;
}

void Prisma::MeshIndirect::updateTextureSize() {
    m_materialData.clear();
    auto meshes = currentGlobalScene->meshes;
    for (auto material : meshes) {
        m_materialData.push_back({ material->material()->diffuse()[0].id(),material->material()->normal()[0].id() ,material->material()->roughness_metalness()[0].id() ,glm::vec2(0.0f) });
    }
    m_ssboMaterial->resize(sizeof(Prisma::MaterialData) * (m_materialData.size()));
    m_ssboMaterial->modifyData(0, sizeof(Prisma::MaterialData) * m_materialData.size(), m_materialData.data());

    m_materialDataAnimation.clear();
    auto meshesAnimation = currentGlobalScene->animateMeshes;
    for (auto material : meshesAnimation) {
        m_materialDataAnimation.push_back({ material->material()->diffuse()[0].id(),material->material()->normal()[0].id() ,material->material()->roughness_metalness()[0].id() ,glm::vec2(0.0f) });
    }
    m_ssboMaterialAnimation->resize(sizeof(Prisma::MaterialData) * (m_materialDataAnimation.size()));
    m_ssboMaterialAnimation->modifyData(0, sizeof(Prisma::MaterialData) * m_materialDataAnimation.size(), m_materialDataAnimation.data());
}
