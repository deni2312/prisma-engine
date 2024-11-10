#include "../../include/Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart()
{
	for (const auto& component : m_components)
	{
		if (!component->isStart())
		{
			component->start();
		}
	}
}

void Prisma::ComponentsHandler::updateUi()
{
	for (const auto& component : m_components)
	{
		if (!component->isUi())
		{
			component->ui();
			component->isUi(true);
		}
	}
}

void Prisma::ComponentsHandler::updateComponents()
{
	for (const auto& component : m_components)
	{
		if (component->isStart())
		{
			component->update();
		}
	}
}

void Prisma::ComponentsHandler::updateRender(std::shared_ptr<FBO> fbo)
{
	for (const auto& component : m_components)
	{
		if (component->isStart())
		{
			component->updateRender(fbo);
		}
	}
}

void Prisma::ComponentsHandler::updatePreRender(std::shared_ptr<FBO> fbo)
{
	for (const auto& component : m_components)
	{
		if (component->isStart())
		{
			component->updatePreRender(fbo);
		}
	}
}

void Prisma::ComponentsHandler::updatePostRender(std::shared_ptr<FBO> fbo)
{
	for (const auto& component : m_components)
	{
		if (component->isStart())
		{
			component->updatePostRender(fbo);
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
		std::cout << "a" << std::endl;
		m_components.erase(it);
	}
}

Prisma::ComponentsHandler::ComponentsHandler()
{
}
