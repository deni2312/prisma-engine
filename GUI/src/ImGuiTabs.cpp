#include "../include/ImGuiTabs.h"
#include <iostream> // For std::cout

#include "Helpers/NodeHelper.h"
#include "SceneObjects/Mesh.h"
#include "GlobalData/CacheScene.h"
#include "../include/ImGuiHelper.h"
#include "ThirdParty/imgui/imgui.h"
#include "ThirdParty/imgui/imgui_internal.h"


#include "GlobalData/GlobalData.h"
#include "Helpers/StringHelper.h"

static char textSearch[128] = "";

Prisma::GUI::ImGuiTabs::ImGuiTabs() {
    m_meshTexture = std::make_shared<Texture>();
    m_meshTexture->name("MeshIcon");
    m_meshTexture->loadTexture({"../../../GUI/icons/mesh.png", false});

    m_lightTexture = std::make_shared<Texture>();
    m_lightTexture->name("LightIcon");
    m_lightTexture->loadTexture({"../../../GUI/icons/light.png", false});

    m_nodeTexture = std::make_shared<Texture>();
    m_nodeTexture->name("NodeIcon");
    m_nodeTexture->loadTexture({"../../../GUI/icons/node.png", false});
}

void Prisma::GUI::ImGuiTabs::updateTabs(std::shared_ptr<Node> root, int depth) {
    if (m_update) {
        m_nodes.clear();
        updateCurrentNodes(root, depth);
        m_update = false;
    }

    if (CacheScene::getInstance().updateSizes() || CacheScene::getInstance().updateLights() ||
        CacheScene::getInstance().updateData()) {
        m_update = true;
    }
}

void Prisma::GUI::ImGuiTabs::dispatch(std::shared_ptr<Node> node, glm::vec2 size) {
    if (std::dynamic_pointer_cast<Mesh>(node)) {
        ImGui::Image(
            m_meshTexture->texture()->GetDefaultView(
                Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE), ImVec2(size.x, size.y));
    } else if (std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node) || std::dynamic_pointer_cast<Light<
                   LightType::LightDir>>(node) || std::dynamic_pointer_cast<Light<
                   LightType::LightArea>>(node)) {
        ImGui::Image(
            m_lightTexture->texture()->GetDefaultView(
                Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE), ImVec2(size.x, size.y));
    } else {
        ImGui::Image(
            m_nodeTexture->texture()->GetDefaultView(
                Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE), ImVec2(size.x, size.y));
    }
}

void Prisma::GUI::ImGuiTabs::showCurrentNodes(ImGuiCamera& camera) {
    std::string currentText = textSearch;
    if (!currentText.empty() && GlobalData::getInstance().currentGlobalScene()->root) {
        std::vector<std::shared_ptr<Node>> findings;
        NodeHelper nodeHelper;
        nodeHelper.nodeIterator(GlobalData::getInstance().currentGlobalScene()->root,
                                [&](auto node, auto parent) {
                                    if (StringHelper::getInstance().toLower(node->name()).find(
                                            StringHelper::getInstance().toLower(currentText)) !=
                                        std::string::npos) {
                                        findings.push_back(node);
                                    }
                                });

        for (int i = 0; i < findings.size(); i++) {
            // Create a button without a label
            std::string finalText = findings[i]->name() + "##" + std::to_string(i);
            if (ImGui::Selectable(finalText.c_str())) {
                camera.currentSelect(findings[i]);
            }

            ImGuiDragDropFlags src_flags = 0;
            src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;
            src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;

            if (ImGui::BeginDragDropSource(src_flags)) {
                m_current = findings[i]->uuid();
                ImGui::SetDragDropPayload("DATA_NAME", &m_current, sizeof(int64_t));
                ImGui::Text(findings[i]->name().c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                ImGuiDragDropFlags target_flags = 0;
                target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
                target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
                // Don't display the yellow rectangle
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                    "DATA_NAME", target_flags)) {
                    m_current = *static_cast<int64_t*>(payload->Data);
                    m_parent = findings[i];
                }
                ImGui::EndDragDropTarget();
            }
        }
    } else {
        auto data = [&](int i) {
            auto child = m_nodes[i].first;
            int depth = m_nodes[i].second + 1;
            // Add spacing based on depth
            ImGui::Indent(depth * 20.0f); // Indent by 20 pixels per depth level
            // Create a button without a label
            std::string finalText = child->name() + "##" + std::to_string(i);
            dispatch(child, glm::vec2(16, 16));
            ImGui::SameLine();
            if (ImGui::Selectable(finalText.c_str())) {
                camera.currentSelect(child);
            }
            ImGuiDragDropFlags src_flags = 0;
            src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;
            src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;

            if (ImGui::BeginDragDropSource(src_flags)) {
                m_current = child->uuid();
                ImGui::SetDragDropPayload("DATA_NAME", &m_current, sizeof(int64_t));
                ImGui::Text(child->name().c_str());
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                ImGuiDragDropFlags target_flags = 0;
                target_flags |= ImGuiDragDropFlags_AcceptBeforeDelivery;
                target_flags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect;
                // Don't display the yellow rectangle
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                    "DATA_NAME", target_flags)) {
                    m_current = *static_cast<int64_t*>(payload->Data);
                    m_parent = child;
                }
                ImGui::EndDragDropTarget();
            }

            // Remove the indent
            ImGui::Unindent(depth * 20.0f);
        };

        ImGuiHelper::getInstance().clipVertical(m_nodes.size(), data);
    }
}

void Prisma::GUI::ImGuiTabs::updateCurrentNodes(std::shared_ptr<Node> root, int depth) {
    if (root) {
        // Iterate through children of the current node
        for (const auto& child : root->children()) {
            m_nodes.push_back({child, depth});
            // Recur for children with increased depth
            updateCurrentNodes(child, depth + 1);
        }
    }
}

static float a = 0;

void Prisma::GUI::ImGuiTabs::showNodes(std::shared_ptr<Node> root, ImGuiCamera& camera) {
    m_current = -1;
    m_parent = nullptr;

    ImGui::InputText("Search", textSearch, IM_ARRAYSIZE(textSearch), // Size of the array (automatically computed)
                     ImGuiInputTextFlags_None, // No special flags
                     nullptr, // No callback
                     nullptr // No user data);
        );
    showCurrentNodes(camera);

    if (m_current && m_parent && m_current == m_parent->uuid()) {
        m_current = -1;
        m_parent = nullptr;
    }

    if (m_current != -1 && m_parent) {
        NodeHelper nodeHelper;
        auto current = nodeHelper.find(m_current);

        auto isMesh = std::dynamic_pointer_cast<Mesh>(current);

        bool isChild = false;

        nodeHelper.nodeIterator(current, [&](auto node, auto parent) {
            if (m_parent->uuid() == node->uuid()) {
                isChild = true;
            }
        });


        if (current && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !isMesh && !isChild) {
            current->parent()->removeChild(m_current, false);

            m_parent->addChild(current);
            current->parent(m_parent, true);
            m_current = -1;
            m_parent = nullptr;
        }
    }
}