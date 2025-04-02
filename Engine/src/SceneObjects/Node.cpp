#include "SceneObjects/Node.h"
#include <iostream>
#include "GlobalData/GlobalData.h"
#include "glm/ext.hpp"
#include "Helpers/PrismaMath.h"
#include "SceneData/MeshIndirect.h"
#include "GlobalData/CacheScene.h"
#include "Helpers/NodeHelper.h"
#include "Helpers/VectorHelper.h"
#include "Handlers/ComponentsHandler.h"

static uint64_t uuidNode = 0;

Prisma::Node::Node() : m_matrix{glm::mat4(1.0f)}, m_finalMatrix{glm::mat4(1.0f)}
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

void Prisma::Node::addChild(std::shared_ptr<Node> child, bool updateScene)
{
	Prisma::GlobalData::getInstance().sceneNodes()[child->uuid()] = child;
	m_children.push_back(child);
	updateChild(this);
	child->parent(this);
	if (updateScene)
	{
		updateCaches(child);
	}
}

void Prisma::Node::removeChild(uint64_t uuid, bool removeRecursive)
{
	NodeHelper nodeHelper;
	int index = nodeHelper.findUUID(m_children, uuid);

	if (index != -1)
	{
		Prisma::GlobalData::getInstance().sceneNodes().erase(Prisma::GlobalData::getInstance().sceneNodes().find(uuid));

		while (m_children[index]->children().size() > 0 && removeRecursive)
		{
			m_children[index]->removeChild(m_children[index]->children()[0]->uuid());
		}
		if (std::dynamic_pointer_cast<AnimatedMesh>(m_children[index]))
		{
			MeshIndirect::getInstance().removeAnimate(index);
			Prisma::VectorHelper::getInstance().remove<AnimatedMesh>(
				Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes, uuid);
			Prisma::AnimationHandler::getInstance().fill();
		}
		else if (std::dynamic_pointer_cast<Mesh>(m_children[index]))
		{
			MeshIndirect::getInstance().remove(index);
			Prisma::VectorHelper::getInstance().remove<Mesh>(
				Prisma::GlobalData::getInstance().currentGlobalScene()->meshes, uuid);
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightDir>>(m_children[index]))
		{
			Prisma::VectorHelper::getInstance().remove<Light<LightType::LightDir>>(
				Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights, uuid);
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightOmni>>(m_children[index]))
		{
			Prisma::VectorHelper::getInstance().remove<Light<LightType::LightOmni>>(
				Prisma::GlobalData::getInstance().currentGlobalScene()->omniLights, uuid);
		}
		else if (std::dynamic_pointer_cast<Sprite>(m_children[index]))
		{
			Prisma::VectorHelper::getInstance().remove<Sprite>(
				Prisma::GlobalData::getInstance().currentGlobalScene()->sprites, uuid);
		}
		else if (std::dynamic_pointer_cast<Light<LightType::LightArea>>(m_children[index]))
		{
			Prisma::VectorHelper::getInstance().remove<Light<LightType::LightArea>>(
				Prisma::GlobalData::getInstance().currentGlobalScene()->areaLights, uuid);
		}

		auto componentsData = m_children[index]->components();

		std::vector<std::string> keys;
		for (const auto& component : componentsData)
		{
			keys.push_back(component.first);
		}

		for (const auto& k : keys) {
			m_children[index]->removeComponent(k);
		}

		m_children[index]->parent(nullptr);
		m_children.erase(m_children.begin() + index);

		CacheScene::getInstance().updateSizes(true);
	}
}

void Prisma::Node::matrix(const glm::mat4& matrix, bool updateChildren)
{
	m_matrix = matrix;
	if (updateChildren)
	{
		auto p = parent();
		glm::mat4 transform(1.0f);
		if (p)
		{
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

void Prisma::Node::parent(Node* parent, bool update)
{
	m_parent = parent;
	if (update)
	{
		updateParent(parent);
	}
	CacheScene::getInstance().updateData(true);
}

Prisma::Node* Prisma::Node::parent() const
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

void Prisma::Node::visible(bool visible)
{
	m_visible = visible;
	CacheScene::getInstance().updateStatus(true);
}

bool Prisma::Node::visible()
{
	return m_visible;
}

Prisma::Node::~Node()
{
}

void Prisma::Node::updateCaches(std::shared_ptr<Node> child)
{
	dispatch(child);
	for (auto c : child->children())
	{
		child->updateCaches(c);
	}
}

void Prisma::Node::updateParent(Node* parent)
{
	if (parent)
	{
		for (auto& child : parent->children())
		{
			for (auto& component : child->components())
			{
				component.second->onParent(parent);
			}
			updateParent(child.get());
		}
	}
}

void Prisma::Node::dispatch(std::shared_ptr<Node> child)
{
	NodeHelper nodeHelper;
	if (std::dynamic_pointer_cast<Mesh>(child) && !std::dynamic_pointer_cast<AnimatedMesh>(child) &&
		child->addGlobalList() && Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size() + 1 < Define::MAX_MESHES)
	{
		if (nodeHelper.findUUID<Mesh>(Prisma::GlobalData::getInstance().currentGlobalScene()->meshes, child->uuid()) <
			0)
		{
			MeshIndirect::getInstance().add(Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size());
			Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.push_back(
				std::dynamic_pointer_cast<Mesh>(child));
			CacheScene::getInstance().updateSizes(true);
		}
	}

	if (std::dynamic_pointer_cast<Light<LightType::LightDir>>(child) &&
		child->addGlobalList() && Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights.size() + 1 < Define::MAX_DIR_LIGHTS)
	{
		if (nodeHelper.findUUID<Light<LightType::LightDir>>(
			Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights, child->uuid()) < 0)
		{
			Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights.push_back(
				std::dynamic_pointer_cast<Light<LightType::LightDir>>(child));
			CacheScene::getInstance().updateLights(true);
		}
	}

	if (std::dynamic_pointer_cast<Light<LightType::LightOmni>>(child) &&
		child->addGlobalList() && Prisma::GlobalData::getInstance().currentGlobalScene()->omniLights.size() + 1 < Define::MAX_DIR_LIGHTS)
	{
		if (nodeHelper.findUUID<Light<LightType::LightOmni>>(
			Prisma::GlobalData::getInstance().currentGlobalScene()->omniLights, child->uuid()) < 0)
		{
			Prisma::GlobalData::getInstance().currentGlobalScene()->omniLights.push_back(
				std::dynamic_pointer_cast<Light<LightType::LightOmni>>(child));
			CacheScene::getInstance().updateLights(true);
		}
	}

	if (std::dynamic_pointer_cast<Sprite>(child) &&
		child->addGlobalList())
	{
		if (nodeHelper.findUUID<Sprite>(Prisma::GlobalData::getInstance().currentGlobalScene()->sprites,
		                                child->uuid()) < 0)
		{
			Prisma::GlobalData::getInstance().currentGlobalScene()->sprites.push_back(
				std::dynamic_pointer_cast<Sprite>(child));
		}
	}

	if (std::dynamic_pointer_cast<Light<LightType::LightArea>>(child) &&
		child->addGlobalList() && Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights.size() + 1 < Define::MAX_AREA_LIGHTS)
	{
		if (nodeHelper.findUUID<Light<LightType::LightArea>>(
			Prisma::GlobalData::getInstance().currentGlobalScene()->areaLights, child->uuid()) < 0)
		{
			Prisma::GlobalData::getInstance().currentGlobalScene()->areaLights.push_back(
				std::dynamic_pointer_cast<Light<LightType::LightArea>>(child));
			CacheScene::getInstance().updateLights(true);
		}
	}

	if (Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size() < Define::MAX_ANIMATION_MESHES)
	{
		if (std::dynamic_pointer_cast<AnimatedMesh>(child) &&
			child->addGlobalList())
		{
			if (nodeHelper.findUUID<AnimatedMesh>(Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes,
			                                      child->uuid()) < 0)
			{
				MeshIndirect::getInstance().addAnimate(
					Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size());
				Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.push_back(
					std::dynamic_pointer_cast<AnimatedMesh>(child));
				CacheScene::getInstance().updateSizes(true);
			}
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


void Prisma::Node::addComponent(std::shared_ptr<Component> component)
{
	if (m_components.find(component->name()) == m_components.end())
	{
		if (m_loadingComponent)
		{
			ComponentsHandler::getInstance().addComponent(component);
		}
		component->parent(this);
		m_components[component->name()] = component;
	}
}

std::map<std::string, std::shared_ptr<Prisma::Component>> Prisma::Node::components()
{
	return m_components;
}

bool Prisma::Node::loadingComponent()
{
	return m_loadingComponent;
}

void Prisma::Node::loadingComponent(bool loadingComponent)
{
	m_loadingComponent = loadingComponent;
}

void Prisma::Node::loadComponents()
{
	for (auto component : m_components)
	{
		ComponentsHandler::getInstance().addComponent(component.second);
	}
	m_loadingComponent = true;
}

void Prisma::Node::addGlobalList(bool globalList)
{
	m_addGlobal = globalList;
}

bool Prisma::Node::addGlobalList() const
{
	return m_addGlobal;
}

void Prisma::Node::removeComponent(const std::string& name)
{
	ComponentsHandler::getInstance().removeComponent(m_components[name]);
	m_components[name]->destroy();
	m_components.erase(name);
}
