#include "../include/MeshInfo.h"
#include "../include/ImGuiDebug.h"
#include "glm/gtx/string_cast.hpp"
#include <tuple>
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "../../Engine/include/Helpers/WindowsHelper.h"
#include "../include/NodeViewer.h"

void Prisma::MeshInfo::showSelected(const NodeViewer::NodeData& meshData)
{
	if (meshData.node)
	{
		auto parent = meshData;
		parent.node = meshData.node->parent();
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

		NodeViewer::getInstance().showSelected(parent, false, false, meshData.node);
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
		ImGui::Dummy(ImVec2(0, 4));

		NodeViewer::getInstance().showComponents(meshData.node);

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
			if (ImGui::Button("Load animation"))
			{
				auto location = Prisma::WindowsHelper::getInstance().openFolder("All Files");
				if (!location.empty()) {
					auto nodeData = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(Prisma::GlobalData::getInstance().sceneNodes()[isAnimate->uuid()]);
					auto animation = std::make_shared<Prisma::Animation>(
						location, nodeData);
					auto animator = std::make_shared<Prisma::Animator>(animation);
					isAnimate->animator(animator);
				}
			}
		}
		ImGui::End();
	}
}

Prisma::MeshInfo::MeshInfo()
{
}
