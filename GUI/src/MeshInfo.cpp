#include "../include/MeshInfo.h"
#include "../include/ImGuiDebug.h"
#include "glm/gtx/string_cast.hpp"
#include <tuple>
#include "../../Engine/include/Components/PhysicsMeshComponent.h"

void Prisma::MeshInfo::drawGizmo(Prisma::MeshInfo::MeshData meshData) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4 model = meshData.mesh->matrix();
    glm::mat4 inverseParent = glm::inverse(meshData.mesh->parent()->finalMatrix());
    ImGuizmo::Manipulate(glm::value_ptr(meshData.camera->matrix()), glm::value_ptr(meshData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(m_rotation), glm::value_ptr(m_scale));

    meshData.mesh->matrix(model);

    //meshData.mesh->finalMatrix(model);

    if (ImGuizmo::IsUsing()) {
        skipUpdate = true;
    }

}

void Prisma::MeshInfo::showSelected(Prisma::MeshInfo::MeshData meshData) {
    if(meshData.mesh) {

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



        ImGui::InputFloat3("Translation",glm::value_ptr(m_translation),"%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Rotation",glm::value_ptr(m_rotation), "%.3f", ImGuiInputTextFlags_ReadOnly);

        ImGui::InputFloat3("Scale", glm::value_ptr(m_scale), "%.3f", ImGuiInputTextFlags_ReadOnly);

        float mass = 0;
        for (auto component : meshData.mesh->components()) {
            auto physicsMesh = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(component);
            if (physicsMesh) {
                mass = physicsMesh->collisionData().mass;
            }
        }
        if (mass <= 0.0) {
            drawGizmo(meshData);
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

void Prisma::MeshInfo::showComponents(MeshData meshData) {
    auto components=meshData.mesh->components();
    for(const auto& component: components){
        ImGui::Separator();
        if(component->isStart()){
            auto fields=component->globalVars();
            ImGui::Text(component->name().c_str());
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
            auto comboData=static_cast<Prisma::ImguiDebug::ImGuiStatus*>(variable);
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
}
