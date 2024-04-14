#include "../include/ImGuiTabs.h"
#include "imgui.h"
#include <iostream> // For std::cout

std::shared_ptr<Prisma::ImGuiTabs> Prisma::ImGuiTabs::instance = nullptr;

Prisma::ImGuiTabs& Prisma::ImGuiTabs::getInstance() {
    if (!instance) {
        instance = std::make_shared<Prisma::ImGuiTabs>();
    }
    return *instance;
}

Prisma::ImGuiTabs::ImGuiTabs() {

}

void Prisma::ImGuiTabs::showCurrentNodes(std::shared_ptr<Node> root, int depth, Prisma::ImGuiCamera& camera)
{
    // Iterate through children of the current node
    for (const auto& child : root->children()) {
        index++;
        // Add spacing based on depth
        ImGui::Indent(depth * 20.0f); // Indent by 20 pixels per depth level

        // Create a button without a label
        if (ImGui::Selectable((child->name() + "##"+std::to_string(index)).c_str())) {
            camera.currentSelect(child.get());
        }

        // Remove the indent
        ImGui::Unindent(depth * 20.0f);

        // Recur for children with increased depth
        showCurrentNodes(child, depth + 1, camera);
    }
}

void Prisma::ImGuiTabs::showNodes(std::shared_ptr<Node> root, int depth, Prisma::ImGuiCamera& camera) {
    showCurrentNodes(root, depth, camera);
    index = 0;
}
