#include "../include/MeshInfo.h"
#include "../include/ImGuiDebug.h"
#include "glm/gtx/string_cast.hpp"
#include <tuple>
#include "../../Engine/include/Components/PhysicsMeshComponent.h"

struct ImGuiMeshInfo {
    std::shared_ptr<Prisma::PhysicsMeshComponent> physicsComponent = nullptr;
};

static ImGuiMeshInfo meshInfoData;

void Prisma::MeshInfo::drawGizmo(const Prisma::MeshInfo::MeshData& meshData) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4 model = meshData.mesh->parent()->finalMatrix();
    glm::mat4 inverseParent=glm::mat4(1.0f);
    if (meshData.mesh->parent()->parent()) {
        inverseParent = glm::inverse(meshData.mesh->parent()->parent()->finalMatrix());
    }

    ImGuizmo::Manipulate(glm::value_ptr(meshData.camera->matrix()), glm::value_ptr(meshData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));

    meshData.mesh->parent()->matrix(inverseParent*model);
    if (meshInfoData.physicsComponent) {
        meshInfoData.physicsComponent->updateCollisionData();
    }
    //meshData.mesh->finalMatrix(model);
}

void Prisma::MeshInfo::showSelected(const Prisma::MeshInfo::MeshData& meshData) {
    if (meshData.mesh) {

        float windowWidth = meshData.translate * meshData.width / 2.0f;
        auto nextRight = [&](float pos) {
            ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
            ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
        };
        nextRight(meshData.initOffset);
        ImGui::Begin(meshData.mesh->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

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

        if (meshData.mesh->visible()) {
            textureId = m_eyeOpen->id();
        }
        else {
            textureId = m_eyeClose->id();
        }

        if (ImGui::ImageButton((void*)textureId, ImVec2(24, 24))) {
            meshData.mesh->visible(!meshData.mesh->visible());
        }

        ImGui::InputFloat3("Translation", glm::value_ptr(m_translation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Rotation", glm::value_ptr(m_rotation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Scale", glm::value_ptr(m_scale), "%.3f", ImGuiInputTextFlags_ReadOnly);

        meshInfoData.physicsComponent = nullptr;
        for (auto component : meshData.mesh->components()) {
            auto physicsMesh = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(component.second);
            if (physicsMesh) {
                meshInfoData.physicsComponent = physicsMesh;
            }
        }
        drawGizmo(meshData);
        auto isAnimate = dynamic_cast<AnimatedMesh*>(meshData.mesh);
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
        showComponents(meshData);
        ImGui::End();
    }
}

std::function<void(glm::mat4 &)> Prisma::MeshInfo::updateMesh() {
    return [&](const glm::mat4& model){
        m_currentModel=model;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));
    };
}

void Prisma::MeshInfo::showComponents(const MeshData& meshData) {
    auto components=meshData.mesh->components();
    for(const auto& component: components){
        ImGui::Separator();
        if(component.second->isUi()){
            auto fields=component.second->globalVars();
            ImGui::Text(component.second->name().c_str());
            for(auto field:fields){
                varsDispatcher(field);
            }
        }
    }}

void Prisma::MeshInfo::varsDispatcher(Prisma::Component::ComponentType types) {
    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    auto type=std::get<0>(types);
    auto name=std::get<1>(types);
    auto variable=std::get<2>(types);

    switch(type){
        case Component::TYPES::BUTTON:
            if(ImGui::Button(name.c_str())){
                (*((std::function<void()>*)variable))();
            }
            break;
        case Component::TYPES::INT:
            ImGui::InputInt(name.c_str(),static_cast<int*>(variable));
            break;
        case Component::TYPES::FLOAT:
            ImGui::InputFloat(name.c_str(),static_cast<float*>(variable));
            break;
        case Component::TYPES::STRING:
            ImGui::Text("%s", ((std::string*)variable)->c_str());
            break;
        case Component::TYPES::BOOL:
            ImGui::Checkbox(name.c_str(),static_cast<bool*>(variable));
            break;
        case Component::TYPES::STRINGLIST:
            auto comboData=static_cast<Prisma::Component::ComponentList*>(variable);
            ImGui::Combo(name.c_str(), &comboData->currentitem, comboData->items.data(), comboData->items.size());
            break;
    }
    ImGui::Dummy(ImVec2(0.0f, 10.0f));
}

Prisma::MeshInfo::MeshInfo()
{
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
