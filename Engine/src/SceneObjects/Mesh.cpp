#include "../../include/SceneObjects/Mesh.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/glm.hpp"
#include "LinearMath/btTransform.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "btBulletDynamicsCommon.h"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_decompose.hpp"

void Prisma::Mesh::loadModel(std::shared_ptr<VerticesData> vertices)
{
	m_vertices = vertices;
}

const Prisma::Mesh::VerticesData& Prisma::Mesh::verticesData() const
{
    return *m_vertices;
}

void Prisma::Mesh::finalMatrix(glm::mat4 matrix, bool update)
{
    Prisma::Node::finalMatrix(matrix);
}

glm::mat4 Prisma::Mesh::finalMatrix() const
{
    return Prisma::Node::finalMatrix();
}

std::shared_ptr<Prisma::Mesh> Prisma::Mesh::instantiate(std::shared_ptr<Mesh> mesh)
{
    auto newInstance = std::make_shared<Mesh>();
    newInstance->loadModel(std::make_shared<VerticesData>(*mesh->m_vertices));
    newInstance->material(std::make_shared<MaterialComponent>(*mesh->material()));
    newInstance->matrix(mesh->matrix());
    return newInstance;
}

void Prisma::Mesh::material(std::shared_ptr<Prisma::MaterialComponent> material)
{
    m_material = material;
    addComponent(m_material);
    updateTextures=true;
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
    m_components.push_back(component);
}

std::vector<std::shared_ptr<Prisma::Component>> Prisma::Mesh::components() {
    return m_components;
}

void Prisma::Mesh::removeComponent(int index) {
    m_components.erase(m_components.begin()+index);
}
