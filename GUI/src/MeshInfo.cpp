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
		std::shared_ptr<PhysicsMeshComponent> physicsComponent = nullptr;
		for (auto component : meshData.node->components())
		{
			auto physicsMesh = std::dynamic_pointer_cast<PhysicsMeshComponent>(component.second);
			if (physicsMesh)
			{
				physicsComponent = physicsMesh;
			}
		}
		auto mesh = dynamic_cast<Prisma::Mesh*>(meshData.node);
		auto isAnimate = dynamic_cast<AnimatedMesh*>(meshData.node);

		NodeViewer::getInstance().showSelected(meshData, false);
		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		if (mesh)
		{
			std::string numberVertices = "";

			if (isAnimate)
			{
				numberVertices = "Vertices: " + std::to_string(isAnimate->animateVerticesData()->vertices.size());
			}
			else
			{
				numberVertices = "Vertices: " + std::to_string(mesh->verticesData().vertices.size());
			}
			ImGui::Text(numberVertices.c_str());
		}
		ImGui::Dummy(ImVec2(0.0f, 4.0f));

		if (physicsComponent)
		{
			physicsComponent->updateCollisionData();
		}

		if (isAnimate || (physicsComponent && !physicsComponent->collisionData().softBody))
		{
			NodeViewer::getInstance().showComponents(meshData.node);
		}
		ImGui::Dummy(ImVec2(0, 10));
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
		ImGui::End();
	}
}

Prisma::MeshInfo::MeshInfo()
{
}
