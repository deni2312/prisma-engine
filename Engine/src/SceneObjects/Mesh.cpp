#include "../../include/SceneObjects/Mesh.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/glm.hpp"
#include "LinearMath/btTransform.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "btBulletDynamicsCommon.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <string>
#include "../../include/GlobalData/CacheScene.h"

void Prisma::Mesh::loadModel(std::shared_ptr<VerticesData> vertices)
{
	m_vertices = vertices;
}

const Prisma::Mesh::VerticesData& Prisma::Mesh::verticesData() const
{
    return *m_vertices;
}

void Prisma::Mesh::matrix(const glm::mat4& matrix, bool update)
{
    Prisma::Node::matrix(matrix);
    Prisma::CacheScene::getInstance().updateData(true);
}

glm::mat4 Prisma::Mesh::matrix() const
{
    return Prisma::Node::matrix();
}

void Prisma::Mesh::finalMatrix(const glm::mat4& matrix, bool update)
{
    Prisma::Node::finalMatrix(matrix);
    Prisma::CacheScene::getInstance().updateData(true);
}

glm::mat4 Prisma::Mesh::finalMatrix() const
{
    return Prisma::Node::finalMatrix();
}

std::shared_ptr<Prisma::Mesh> Prisma::Mesh::instantiate(std::shared_ptr<Mesh> mesh)
{
    std::shared_ptr<Mesh> newInstance = nullptr;
    if (mesh) {
        newInstance = std::make_shared<Mesh>();
        newInstance->loadModel(std::make_shared<VerticesData>(*mesh->m_vertices));
        newInstance->material(std::make_shared<MaterialComponent>(*mesh->material()));
        newInstance->matrix(mesh->matrix());
        newInstance->name(mesh->name()+std::to_string(newInstance->uuid()));
        std::shared_ptr<Node> parent = std::make_shared<Node>();
        parent->name(mesh->parent()->name() + std::to_string(parent->uuid()));
        parent->matrix(mesh->parent()->matrix());
        currentGlobalScene->root->addChild(parent);
        parent->addChild(newInstance);
    }
    return newInstance;
}

void Prisma::Mesh::material(std::shared_ptr<Prisma::MaterialComponent> material)
{
    m_material = material;
    addComponent(m_material);
    Prisma::CacheScene::getInstance().updateTextures(true);
}

std::shared_ptr<Prisma::MaterialComponent> Prisma::Mesh::material()
{
    return m_material;
}

void Prisma::Mesh::vectorId(unsigned int vectorId)
{
    m_vectorId = vectorId;
}

unsigned int Prisma::Mesh::vectorId()
{
    return m_vectorId;
}

void Prisma::Mesh::computeAABB() {

    auto vertices=verticesData().vertices;
    auto indices=verticesData().indices;
    if (vertices.empty()) {
        m_aabbData=AABBData{glm::vec3(0.0f), glm::vec3(0.0f)};
    }

    glm::vec3 minPoint = vertices[0].position;
    glm::vec3 maxPoint = vertices[0].position;

    for (const auto& vertex : vertices) {
        minPoint = glm::min(minPoint, vertex.position);
        maxPoint = glm::max(maxPoint, vertex.position);
    }

    m_aabbData.min= minPoint;
    m_aabbData.max= maxPoint;

    glm::vec3 center(0.0f);

    for (const auto& index : indices) {
        center += vertices[index].position;
    }

    center /= static_cast<float>(vertices.size());

    m_aabbData.center=center;
}

Prisma::Mesh::AABBData Prisma::Mesh::aabbData() {
    return m_aabbData;
}

void Prisma::Mesh::addComponent(std::shared_ptr<Prisma::Component> component) {
    component->parent(this);
    m_components[component->name()]=component;
}

std::map<std::string,std::shared_ptr<Prisma::Component>> Prisma::Mesh::components() {
    return m_components;
}

void Prisma::Mesh::removeComponent(const std::string& name) {
    m_components.erase(name);
}
