#include "../include/AddingMenu.h"
#include "ThirdParty/imgui/imgui.h"

void Prisma::GUI::ImGuiAddingMenu::addMenu(ImGuiCamera& camera) const {
    if (camera.openPopup()) {
        ImGui::OpenPopup("RightClickMenu");
        camera.openPopup(false);
    }
    // Right-click context menu
    if (ImGui::BeginPopup("RightClickMenu")) {
        if (ImGui::MenuItem("Add Node")) {
            NodeCreator::createNode();
        }
        if (ImGui::MenuItem("Add Cube")) {
            NodeCreator::createCube();
        }
        if (ImGui::MenuItem("Add Sphere")) {
            NodeCreator::createSphere(m_subDivisions);
        }
        if (ImGui::MenuItem("Add Omni")) {
            NodeCreator::createOmnidirectional();
        }
        if (ImGui::MenuItem("Add Directional")) {
            NodeCreator::createDirectional();
        }
        if (ImGui::MenuItem("Add Spot")) {
            NodeCreator::createSpot();
        }

        ImGui::EndPopup();
    }
}