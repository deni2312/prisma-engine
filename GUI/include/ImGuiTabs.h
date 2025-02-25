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
		void showNodes(std::shared_ptr<Node> root, ImGuiCamera& camera);

		ImGuiTabs();

		void updateTabs(std::shared_ptr<Node> root, int depth);

	private:
		void updateCurrentNodes(std::shared_ptr<Node> root, int depth);
		void showCurrentNodes(ImGuiCamera& camera);

		int64_t m_current = -1;
		bool m_update = false;
		std::shared_ptr<Prisma::Node> m_parent = nullptr;

		std::vector<std::pair<std::shared_ptr<Prisma::Node>,int>> m_nodes;
	};
}
