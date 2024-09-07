#pragma once
#include "../../Engine/include/GlobalData/InstanceData.h"
#include "../../Engine/include/Components/Component.h"
#include <imgui.h>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include "../../Engine/include/SceneObjects/Camera.h"
#include "../../Engine/include/Containers/Texture.h"
#include <ImGuizmo.h>
#include <iostream>

namespace Prisma {
	class NodeViewer : public Prisma::InstanceData<NodeViewer>{
	public:
        struct NodeData {
            Prisma::Node* node;
            std::shared_ptr<Prisma::Camera> camera;
            glm::mat4 projection;
            float translate;
            float width;
            float height;
            float scale;
            float initOffset;
        };

        void varsDispatcher(Prisma::Component::ComponentType types);

        NodeViewer() {
            m_rotateTexture = std::make_shared<Prisma::Texture>();
            m_rotateTexture->loadTexture("../../../GUI/icons/rotate.png", false, false, false);

            m_translateTexture = std::make_shared<Prisma::Texture>();
            m_translateTexture->loadTexture("../../../GUI/icons/move.png", false, false, false);

            m_scaleTexture = std::make_shared<Prisma::Texture>();
            m_scaleTexture->loadTexture("../../../GUI/icons/scale.png", false, false, false);

            m_eyeOpen = std::make_shared<Prisma::Texture>();
            m_eyeOpen->loadTexture("../../../GUI/icons/eyeopen.png", false, false, false);

            m_eyeClose = std::make_shared<Prisma::Texture>();
            m_eyeClose->loadTexture("../../../GUI/icons/eyeclose.png", false, false, false);
        }

        void showComponents(Node* nodeData) {
            auto components = nodeData->components();
            for (const auto& component : components) {
                ImGui::Separator();
                if (component.second->isUi()) {
                    auto fields = component.second->globalVars();
                    ImGui::Text(component.second->name().c_str());
                    for (auto field : fields) {
                        Prisma::NodeViewer::getInstance().varsDispatcher(field);
                    }
                }
            }
        }

        void showSelected(const NodeData& nodeData) {
            if (nodeData.node) {

                float windowWidth = nodeData.translate * nodeData.width / 2.0f;
                auto nextRight = [&](float pos) {
                    ImGui::SetNextWindowPos(ImVec2(windowWidth + nodeData.scale * nodeData.width, pos));
                    ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
                };
                nextRight(nodeData.initOffset);
                ImGui::Begin(nodeData.node->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

                if (ImGui::ImageButton((void*)m_rotateTexture->id(), ImVec2(24, 24))) {
                    mCurrentGizmoOperation = ImGuizmo::ROTATE;
                }
                ImGui::SameLine();

                if (ImGui::ImageButton((void*)m_translateTexture->id(), ImVec2(24, 24))) {
                    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                }
                ImGui::SameLine();

                if (ImGui::ImageButton((void*)m_scaleTexture->id(), ImVec2(24, 24))) {
                    mCurrentGizmoOperation = ImGuizmo::SCALE;
                }
                ImGui::SameLine();

                auto textureId = 0;

                if (nodeData.node->visible()) {
                    textureId = m_eyeOpen->id();
                }
                else {
                    textureId = m_eyeClose->id();
                }

                if (ImGui::ImageButton((void*)textureId, ImVec2(24, 24))) {
                    nodeData.node->visible(!nodeData.node->visible());
                }

                ImGui::InputFloat3("Translation", glm::value_ptr(m_translation), "%.3f", ImGuiInputTextFlags_ReadOnly);

                ImGui::InputFloat3("Rotation", glm::value_ptr(m_rotation), "%.3f", ImGuiInputTextFlags_ReadOnly);

                ImGui::InputFloat3("Scale", glm::value_ptr(m_scale), "%.3f", ImGuiInputTextFlags_ReadOnly);

                drawGizmo(nodeData);

                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                showComponents(nodeData.node);
                ImGui::End();
            }
        }
        // Getters for textures
        const std::shared_ptr<Prisma::Texture>& rotateTexture() const {
            return m_rotateTexture;
        }

        const std::shared_ptr<Prisma::Texture>& translateTexture() const {
            return m_translateTexture;
        }

        const std::shared_ptr<Prisma::Texture>& scaleTexture() const {
            return m_scaleTexture;
        }

        const std::shared_ptr<Prisma::Texture>& eyeOpenTexture() const {
            return m_eyeOpen;
        }

        const std::shared_ptr<Prisma::Texture>& eyeCloseTexture() const {
            return m_eyeClose;
        }

    private:
        glm::vec3 m_scale;
        glm::vec3 m_rotation;
        glm::vec3 m_translation;

        glm::mat4 m_currentModel;

        std::shared_ptr<Prisma::Texture> m_rotateTexture;
        std::shared_ptr<Prisma::Texture> m_translateTexture;
        std::shared_ptr<Prisma::Texture> m_scaleTexture;
        std::shared_ptr<Prisma::Texture> m_eyeOpen;
        std::shared_ptr<Prisma::Texture> m_eyeClose;

        ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;


        void drawGizmo(const NodeData& nodeData) {
            ImGuiIO& io = ImGui::GetIO();
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
            glm::mat4 model = nodeData.node->parent()->finalMatrix();
            glm::mat4 inverseParent = glm::mat4(1.0f);
            if (nodeData.node->parent()->parent()) {
                inverseParent = glm::inverse(nodeData.node->parent()->parent()->finalMatrix());
            }

            ImGuizmo::Manipulate(glm::value_ptr(nodeData.camera->matrix()), glm::value_ptr(nodeData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));

            nodeData.node->parent()->matrix(inverseParent * model);
        }
    };
}