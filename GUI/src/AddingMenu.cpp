#include "../include/AddingMenu.h"

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