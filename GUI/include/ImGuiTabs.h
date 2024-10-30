#pragma once

#include <memory>
#include "../../Engine/include/SceneObjects/Node.h"
#include "ImGuiCamera.h"
#include "../../Engine/include/Containers/Texture.h"
#include "../../Engine/include/GlobalData/InstanceData.h"

namespace Prisma
{
	class ImGuiTabs : public InstanceData<ImGuiTabs>
	{
	public:
		void showNodes(std::shared_ptr<Node> root, int depth, ImGuiCamera& camera);

		ImGuiTabs();

	private:
		void showCurrentNodes(std::shared_ptr<Node> root, int depth, ImGuiCamera& camera);

		unsigned long int m_index = 0;
	};
}
