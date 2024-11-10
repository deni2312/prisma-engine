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

		void updatePostRender(std::shared_ptr<FBO> fbo = nullptr);

		void addComponent(std::shared_ptr<Component> component);

		void removeComponent(std::shared_ptr<Component> component);

		ComponentsHandler();

	private:
		std::vector<std::shared_ptr<Component>> m_components;
	};
}
