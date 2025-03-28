#include "../include/AddingMenu.h"
#include "ThirdParty/imgui/imgui.h"

void Prisma::ImGuiAddingMenu::addMenu(Prisma::ImGuiCamera& camera) const
{
	if (camera.openPopup())
	{
		ImGui::OpenPopup("RightClickMenu");
		camera.openPopup(false);
	}
	// Right-click context menu
	if (ImGui::BeginPopup("RightClickMenu"))
	{
		if (ImGui::MenuItem("Add Node"))
		{
			Prisma::NodeCreator::createNode();
		}
		if (ImGui::MenuItem("Add Cube"))
		{
			Prisma::NodeCreator::createCube();
		}
		if (ImGui::MenuItem("Add Sphere"))
		{
			Prisma::NodeCreator::createSphere(m_subDivisions);
		}
		if (ImGui::MenuItem("Add Omni"))
		{
			Prisma::NodeCreator::createOmnidirectional();
		}
		if (ImGui::MenuItem("Add Directional"))
		{
			Prisma::NodeCreator::createDirectional();
		}
		if (ImGui::MenuItem("Add Area"))
		{
			Prisma::NodeCreator::createArea();
		}
		ImGui::EndPopup();
	}
}
