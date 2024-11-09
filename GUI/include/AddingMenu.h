#pragma once
#include "ImGuiCamera.h"
#include <imgui.h>
#include "MeshCreator.h"

namespace Prisma
{
	class ImGuiAddingMenu
	{
	public:
		void addMenu(Prisma::ImGuiCamera& camera)
		{
			if (camera.openPopup())
			{
				ImGui::OpenPopup("RightClickMenu");
				camera.openPopup(false);
			}
			// Right-click context menu
			if (ImGui::BeginPopup("RightClickMenu"))
			{
				if (ImGui::MenuItem("Add Cube"))
				{
					Prisma::MeshCreator::createCube();
				}
				if (ImGui::MenuItem("Add Sphere"))
				{
					Prisma::MeshCreator::createSphere(m_subDivisions);
				}
				ImGui::EndPopup();
			}
		}

	private:
		const int m_subDivisions = 16;
	};
}
