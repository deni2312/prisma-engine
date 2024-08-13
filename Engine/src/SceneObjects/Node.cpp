#include "../../include/SceneObjects/Node.h"
#include <iostream>
#include "../../include/GlobalData/GlobalData.h"
#include "glm/ext.hpp"
#include "../../include/Helpers/PrismaMath.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/GlobalData/CacheScene.h"

static uint64_t uuidNode = 0;

Prisma::Node::Node() :m_matrix{ glm::mat4(1.0f) },m_finalMatrix{glm::mat4(1.0f)}
{
	m_uuid = uuidNode;
	uuidNode = uuidNode + 1;
}

void Prisma::Node::name(const std::string& name)
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

void Prisma::Node::addChild(std::shared_ptr<Prisma::Node> child, bool updateScene, bool recursiveAdd)
{
	m_children.push_back(child);
	if (updateScene) {
		dispatch(child);

		if (recursiveAdd) {
			updateCaches(child);
		}

	}
}

void Prisma::Node::removeChild(uint64_t uuid)
{
	unsigned int index = -1;
	for (int i = 0; i < m_children.size(); i++) {
		if (m_children[i]->uuid() == uuid) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		while(m_children[index]->children().size()>0) {
			m_children[index]->removeChild(m_children[index]->children()[0]->uuid());
		}
		if (std::dynamic_pointer_cast<Prisma::AnimatedMesh>(m_children[index])) {
			MeshIndirect::getInstance().removeAnimate(index);
			auto find = std::find_if(currentGlobalScene->animateMeshes.begin(), currentGlobalScene->animateMeshes.end(), [uuid](auto mesh) {
				if (mesh->uuid() == uuid) {
					return true;
				}
				return false;
				});
			auto components = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(m_children[index])->components();
			for (const auto& component : components) {
				component.second->destroy();
			}
			currentGlobalScene->animateMeshes.erase(find);
		}
		else if(std::dynamic_pointer_cast<Prisma::Mesh>(m_children[index])){
			MeshIndirect::getInstance().remove(index);
			auto find = std::find_if(currentGlobalScene->meshes.begin(), currentGlobalScene->meshes.end(), [uuid](auto mesh) {
				if (mesh->uuid() == uuid) {
					return true;
				}
				return false;
				});
			auto components = std::dynamic_pointer_cast<Prisma::Mesh>(m_children[index])->components();
			for (const auto& component : components) {
				component.second->destroy();
			}
			currentGlobalScene->meshes.erase(find);
		}
		else if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(m_children[index])) {
			auto find = std::find_if(currentGlobalScene->dirLights.begin(), currentGlobalScene->dirLights.end(), [uuid](auto light) {
				if (light->uuid() == uuid) {
					return true;
				}
				return false;
				});
			currentGlobalScene->dirLights.erase(find);
		}
		else if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(m_children[index])) {
			auto find = std::find_if(currentGlobalScene->omniLights.begin(), currentGlobalScene->omniLights.end(), [uuid](auto light) {
				if (light->uuid() == uuid) {
					return true;
				}
				return false;
				});
			currentGlobalScene->omniLights.erase(find);
		}


		m_children.erase(m_children.begin()+index);
		Prisma::CacheScene::getInstance().updateSizes(true);
	}
}

void Prisma::Node::matrix(const glm::mat4& matrix, bool updateChildren)
{
    m_matrix = matrix;
    if (updateChildren) {
        auto p = parent();
        glm::mat4 transform(1.0f);
        if (p) {
            transform = p->finalMatrix();
        }
        finalMatrix(transform * m_matrix);
        updateChild(this);
    }
}

glm::mat4 Prisma::Node::matrix() const
{
	return m_matrix;
}

void Prisma::Node::finalMatrix(const glm::mat4& matrix, bool update)
{
    m_finalMatrix = matrix;
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

void Prisma::Node::visible(bool visible) {
	m_visible = visible;
	Prisma::CacheScene::getInstance().updateStatus(true);
}

bool Prisma::Node::visible() {
	return m_visible;
}

Prisma::Node::~Node()
{
}

void Prisma::Node::updateCaches(std::shared_ptr<Node> child) {
	dispatch(child);
	for (auto c : child->children()) {
		child->updateCaches(c);
	}
}

void Prisma::Node::dispatch(std::shared_ptr<Node> child)
{
	if (std::dynamic_pointer_cast<Prisma::Mesh>(child) && !std::dynamic_pointer_cast<Prisma::AnimatedMesh>(child)) {
		MeshIndirect::getInstance().add(currentGlobalScene->meshes.size());
		currentGlobalScene->meshes.push_back(std::dynamic_pointer_cast<Mesh>(child));
		Prisma::CacheScene::getInstance().updateSizes(true);
	}
	if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(child)) {
		currentGlobalScene->dirLights.push_back(std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(child));
		Prisma::CacheScene::getInstance().updateLights(true);
	}
	if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(child)) {
		currentGlobalScene->omniLights.push_back(std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(child));
		Prisma::CacheScene::getInstance().updateLights(true);
	}
	if (currentGlobalScene->animateMeshes.size() < MAX_ANIMATION_MESHES) {
		if (std::dynamic_pointer_cast<Prisma::AnimatedMesh>(child)) {
			MeshIndirect::getInstance().addAnimate(currentGlobalScene->animateMeshes.size());
			currentGlobalScene->animateMeshes.push_back(std::dynamic_pointer_cast<AnimatedMesh>(child));
			Prisma::CacheScene::getInstance().updateSizes(true);
		}
	}
	else
	{
		std::cerr << "MAX ANIMATION MESHES REACHED" << std::endl;
	}
}

void Prisma::Node::updateChild(Node* node)
{
	for (unsigned int i = 0; i < node->children().size(); i++)
	{
		node->children()[i]->finalMatrix(node->finalMatrix() * node->children()[i]->matrix());
		updateChild(node->children()[i].get());
	}
}
