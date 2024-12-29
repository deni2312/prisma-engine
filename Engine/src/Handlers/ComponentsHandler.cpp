#include "../../include/Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart()
{
	for (const auto& component : m_components)
	{
		if (component && !component->isStart())
		{
			component->start();
		}
		if (!component) {
			std::cout << component << std::endl;
		}
	}
}

void Prisma::ComponentsHandler::updateUi()
{
	for (const auto& component : m_components)
	{
		if (component && !component->isUi())
		{
			component->ui();
			component->isUi(true);
		}
		if (!component) {
			std::cout << component << std::endl;
		}
	}
}

void Prisma::ComponentsHandler::updateComponents()
{
	for (const auto& component : m_components)
	{
		if (component && component->isStart())
		{
			component->update();
		}
		if (!component) {
			std::cout << component << std::endl;
		}
	}
}

void Prisma::ComponentsHandler::updateRender(std::shared_ptr<FBO> fbo)
{
	for (const auto& component : m_components)
	{
		if (component && component->isStart())
		{
			component->updateRender(fbo);
		}
		if (!component) {
			std::cout << component << std::endl;
		}
	}
}

void Prisma::ComponentsHandler::updatePreRender(std::shared_ptr<FBO> fbo)
{
	for (const auto& component : m_components)
	{
		if (component && component->isStart())
		{
			component->updatePreRender(fbo);
		}
		if (!component) {
			std::cout << component << std::endl;
		}
	}
}

void Prisma::ComponentsHandler::updatePostRender(std::shared_ptr<FBO> fbo)
{
	for (const auto& component : m_components)
	{
		if (component && component->isStart())
		{
			component->updatePostRender(fbo);
		}
		if (!component) {
			std::cout << component << std::endl;
		}
	}
}

void Prisma::ComponentsHandler::addComponent(std::shared_ptr<Component> component)
{
	m_components.push_back(component);
}

void Prisma::ComponentsHandler::removeComponent(std::shared_ptr<Component> component)
{
	// Find and remove the component from the vector
	auto it = std::find(m_components.begin(), m_components.end(), component);

	// Erase the removed elements (if any)
	if (it != m_components.end())
	{
		m_components.erase(it);
	}
}

Prisma::ComponentsHandler::ComponentsHandler()
{
	m_registerData.init();
}
