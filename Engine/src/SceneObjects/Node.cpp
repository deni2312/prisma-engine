#include "../../include/SceneObjects/Node.h"
#include <iostream>
#include "../../include/GlobalData/GlobalData.h"
#include "glm/ext.hpp"
#include "../../include/Helpers/PrismaMath.h"

static uint64_t uuidNode = 0;

Prisma::Node::Node() :m_matrix{ glm::mat4(1.0f) },m_finalMatrix{glm::mat4(1.0f)}
{
	m_uuid = uuidNode;
	uuidNode = uuidNode + 1;
}

void Prisma::Node::name(std::string name)
{
	m_name = name;
}

std::string Prisma::Node::name() const
{
	return m_name;
}

const std::vector<std::shared_ptr<Prisma::Node>>& Prisma::Node::children() const
{
	return m_children;
}

void Prisma::Node::addChild(std::shared_ptr<Prisma::Node> child, bool updateScene)
{
	m_children.push_back(child);
	if (updateScene) {
		if (std::dynamic_pointer_cast<Prisma::Mesh>(child)) {
			currentGlobalScene->meshes.push_back(std::dynamic_pointer_cast<Mesh>(child));
		}
		if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(child)) {
			currentGlobalScene->dirLights.push_back(std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(child));
		}
		if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(child)) {
			currentGlobalScene->omniLights.push_back(std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(child));
		}
		if (std::dynamic_pointer_cast<Prisma::AnimatedMesh>(child)) {
			currentGlobalScene->animateMeshes.push_back(std::dynamic_pointer_cast<AnimatedMesh>(child));
		}
	}
	updateSizes = true;
}

void Prisma::Node::removeChild(uint64_t uuid)
{
	auto it = std::find_if(m_children.begin(), m_children.end(),
		[&](std::shared_ptr<Prisma::Node> node)
        {
            if(node->uuid() == uuid){
                node->parent(nullptr);
                return true;
            }
            return false;
        });

	if (it != m_children.end())
	{
		m_children.erase(it);
	}
	updateSizes = true;
}

void Prisma::Node::matrix(glm::mat4 matrix, bool updateChildren)
{
    if(!Prisma::mat4Equals(m_matrix,matrix)) {
        m_matrix = matrix;
        updateData = true;
        if (updateChildren) {
            auto p = parent();
            glm::mat4 transform(1.0f);
            if (p) {
                transform = p->finalMatrix();
            }
			std::cout <<name()<< p->name() << std::endl;
            finalMatrix(transform * m_matrix);
            updateChild(this);
        }
    }
}

glm::mat4 Prisma::Node::matrix() const
{
	return m_matrix;
}

void Prisma::Node::finalMatrix(glm::mat4 matrix, bool update)
{
    if(!Prisma::mat4Equals(m_finalMatrix,matrix)) {
        m_finalMatrix = matrix;
        updateData = true;
    }
}

glm::mat4 Prisma::Node::finalMatrix() const
{
	return m_finalMatrix;
}

void Prisma::Node::parent(std::shared_ptr<Node> parent)
{
	m_parent = parent;
}

std::shared_ptr<Prisma::Node> Prisma::Node::parent() const
{
	return m_parent;
}

void Prisma::Node::istantiate(std::shared_ptr<Node> node)
{
}

uint64_t Prisma::Node::uuid()
{
	return m_uuid;
}

Prisma::Node::~Node()
{
}

void Prisma::Node::updateChild(Node* node)
{
	for (unsigned int i = 0; i < node->children().size(); i++)
	{
		node->children()[i]->finalMatrix(node->finalMatrix() * node->children()[i]->matrix());
		updateChild(node->children()[i].get());
	}
}
