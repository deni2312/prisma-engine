#include "../include/ImGuiTabs.h"
#include "imgui.h"
#include <iostream> // For std::cout

Prisma::ImGuiTabs::ImGuiTabs() {

}

void Prisma::ImGuiTabs::showCurrentNodes(std::shared_ptr<Node> root, int depth, Prisma::ImGuiCamera& camera)
{
    if (root) {
        // Iterate through children of the current node
        for (const auto& child : root->children()) {
            m_index++;
            // Add spacing based on depth
            ImGui::Indent(depth * 20.0f); // Indent by 20 pixels per depth level
            // Create a button without a label
            std::string finalText = child->name() + "##" + std::to_string(m_index);
            if (ImGui::Selectable(finalText.c_str())) {
                camera.currentSelect(child.get());
            }
            // Remove the indent
            ImGui::Unindent(depth * 20.0f);
            // Recur for children with increased depth
            showCurrentNodes(child, depth + 1, camera);
        }
    }
}

void Prisma::ImGuiTabs::showNodes(std::shared_ptr<Node> root, int depth, Prisma::ImGuiCamera& camera) {
    showCurrentNodes(root, depth, camera);
    m_index = 0;
}
