#pragma once

#include "../GlobalData/GlobalData.h"
#include "../Components/Component.h"
#include "../GlobalData/InstanceData.h"


namespace Prisma
{
	class ComponentsHandler : public InstanceData<ComponentsHandler>
	{
	public:
		void updateStart();

		void updateUi();

		void updateComponents();

		void updateRender(std::shared_ptr<FBO> fbo = nullptr);

		void updatePreRender(std::shared_ptr<FBO> fbo = nullptr);

		void addComponent(std::shared_ptr<Component> component)
		{
			m_components.push_back(component);
		}

		void removeComponent(std::shared_ptr<Component> component)
		{
			// Find and remove the component from the vector
			auto it = std::remove(m_components.begin(), m_components.end(), component);

			// Erase the removed elements (if any)
			if (it != m_components.end())
			{
				m_components.erase(it, m_components.end());
			}
		}

		ComponentsHandler();

	private:
		std::vector<std::shared_ptr<Component>> m_components;
	};
}
