#include "../include/MeshInfo.h"
#include "../include/ImGuiDebug.h"
#include "glm/gtx/string_cast.hpp"
#include <tuple>
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "../include/NodeViewer.h"

void Prisma::MeshInfo::showSelected(const NodeViewer::NodeData& meshData)
{
	if (meshData.node)
	{
		NodeViewer::NodeData parentData = meshData;
		parentData.node = parentData.node->parent();

		NodeViewer::getInstance().showSelected(parentData, false);

		std::shared_ptr<PhysicsMeshComponent> physicsComponent = nullptr;
		for (auto component : meshData.node->components())
		{
			auto physicsMesh = std::dynamic_pointer_cast<PhysicsMeshComponent>(component.second);
			if (physicsMesh)
			{
				physicsComponent = physicsMesh;
			}
		}
		if (physicsComponent)
		{
			physicsComponent->updateCollisionData();
		}
		auto isAnimate = dynamic_cast<AnimatedMesh*>(meshData.node);
		if (isAnimate)
		{
			auto animator = isAnimate->animator();
			if (animator)
			{
				auto animation = animator->animation();
				auto getLast = [](std::string s)
				{
					size_t found = s.find_last_of('/');
					return found != std::string::npos ? s.substr(found + 1) : s;
				};
				auto name = getLast(animation->name());
				ImGui::Text("%s", name.c_str());
				float current = animator->currentTime();
				//ImGui::ProgressBar(current);
				if (ImGui::SliderFloat("Frames", &current, 0.0f, animation->duration()))
				{
					animator->frame(current);
				}
			}
		}
		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		NodeViewer::getInstance().showComponents(meshData.node);
		ImGui::End();
	}
}

Prisma::MeshInfo::MeshInfo()
{
}
