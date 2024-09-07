#include "../include/MeshInfo.h"
#include "../include/ImGuiDebug.h"
#include "glm/gtx/string_cast.hpp"
#include <tuple>
#include "../../Engine/include/Components/PhysicsMeshComponent.h"
#include "../include/NodeViewer.h"

struct ImGuiMeshInfo {
    std::shared_ptr<Prisma::PhysicsMeshComponent> physicsComponent = nullptr;
};

static ImGuiMeshInfo meshInfoData;

void Prisma::MeshInfo::drawGizmo(const Prisma::NodeViewer::NodeData& meshData) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4 model = meshData.node->parent()->finalMatrix();
    glm::mat4 inverseParent=glm::mat4(1.0f);
    if (meshData.node->parent()->parent()) {
        inverseParent = glm::inverse(meshData.node->parent()->parent()->finalMatrix());
    }

    ImGuizmo::Manipulate(glm::value_ptr(meshData.camera->matrix()), glm::value_ptr(meshData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));

    meshData.node->parent()->matrix(inverseParent*model);
    if (meshInfoData.physicsComponent) {
        meshInfoData.physicsComponent->updateCollisionData();
    }
    //meshData.mesh->finalMatrix(model);
}

void Prisma::MeshInfo::showSelected(const Prisma::NodeViewer::NodeData& meshData) {
    if (meshData.node) {

        float windowWidth = meshData.translate * meshData.width / 2.0f;
        auto nextRight = [&](float pos) {
            ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, meshData.height * meshData.scale - pos));
        };
        nextRight(meshData.initOffset);
        ImGui::Begin(meshData.node->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::ImageButton((void*)Prisma::NodeViewer::getInstance().rotateTexture()->id(), ImVec2(24, 24))) {
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();

        if (ImGui::ImageButton((void*)Prisma::NodeViewer::getInstance().translateTexture()->id(), ImVec2(24, 24))) {
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();

        if (ImGui::ImageButton((void*)Prisma::NodeViewer::getInstance().scaleTexture()->id(), ImVec2(24, 24))) {
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        }
        ImGui::SameLine();

        auto textureId = 0;

        if (meshData.node->visible()) {
            textureId = Prisma::NodeViewer::getInstance().eyeOpenTexture()->id();
        }
        else {
            textureId = Prisma::NodeViewer::getInstance().eyeCloseTexture()->id();
        }

        if (ImGui::ImageButton((void*)textureId, ImVec2(24, 24))) {
            meshData.node->visible(!meshData.node->visible());
        }

        ImGui::InputFloat3("Translation", glm::value_ptr(m_translation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Rotation", glm::value_ptr(m_rotation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Scale", glm::value_ptr(m_scale), "%.3f", ImGuiInputTextFlags_ReadOnly);

        meshInfoData.physicsComponent = nullptr;
        for (auto component : meshData.node->components()) {
            auto physicsMesh = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(component.second);
            if (physicsMesh) {
                meshInfoData.physicsComponent = physicsMesh;
            }
        }
        drawGizmo(meshData);
        auto isAnimate = dynamic_cast<AnimatedMesh*>(meshData.node);
        if (isAnimate) {
            auto animator = isAnimate->animator();
            if (animator) {
                auto animation = animator->animation();
                auto getLast = [](std::string s) {
                    size_t found = s.find_last_of('/');
                    return found != std::string::npos ? s.substr(found + 1) : s;
                };
                auto name = getLast(animation->name());
                ImGui::Text("%s", name.c_str());
                float current = animator->currentTime();
                //ImGui::ProgressBar(current);
                if (ImGui::SliderFloat("Frames", &current, 0.0f, animation->duration())) {
                    animator->frame(current);
                }
            }
        }
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        Prisma::NodeViewer::getInstance().showComponents(meshData.node);
        ImGui::End();
    }
}

std::function<void(glm::mat4 &)> Prisma::MeshInfo::updateMesh() {
    return [&](const glm::mat4& model){
        m_currentModel=model;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));
    };
}

Prisma::MeshInfo::MeshInfo()
{

}
