#include "../include/MeshInfo.h"
#include "../include/ImGuiDebug.h"
#include "glm/gtx/string_cast.hpp"
#include <tuple>
#include "../../Engine/include/Components/PhysicsMeshComponent.h"

void Prisma::MeshInfo::drawGizmo(Prisma::MeshInfo::MeshData meshData) {
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::mat4 model = meshData.mesh->matrix();
    glm::mat4 delta;
    ImGuizmo::Manipulate(glm::value_ptr(meshData.camera->matrix()), glm::value_ptr(meshData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model), glm::value_ptr(delta));

    glm::vec3 rotation;
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(delta), glm::value_ptr(m_translation), glm::value_ptr(rotation), glm::value_ptr(m_scale));
    m_rotation = glm::quat(glm::radians(glm::degrees(glm::eulerAngles(m_rotation))+rotation));
    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(m_translation), glm::value_ptr(rotation), glm::value_ptr(m_scale));

    glm::vec3 halfExtents = (meshData.mesh->aabbData().max - meshData.mesh->aabbData().min) * 0.5f;
    glm::vec3 origin(meshData.mesh->aabbData().min.x + halfExtents.x, meshData.mesh->aabbData().min.y + halfExtents.y,
        meshData.mesh->aabbData().min.z + halfExtents.z);
    glm::mat4 PrismaMatrix = glm::translate(glm::mat4(1.0f), m_translation + origin);

    auto scalingVector = PrismaMatrix[3] * (glm::vec4(m_scale - glm::vec3(1.0f), 1.0));

    glm::mat4 trans_to_pivot = glm::translate(glm::mat4(1.0f), -(m_translation + origin));
    glm::mat4 trans_from_pivot = glm::translate(glm::mat4(1.0f), m_translation + origin);
    glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(m_rotation));
    glm::vec3 eulerRotation1 = glm::radians(eulerRotation);
    glm::mat4 finalRotation = glm::rotate(glm::mat4(1.0f), eulerRotation1.x, glm::vec3(1.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), eulerRotation1.y, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), eulerRotation1.z, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 rotate_matrix = finalRotation;

    glm::mat4 resultRotation = trans_from_pivot * rotate_matrix * trans_to_pivot;

    PrismaMatrix = glm::scale(glm::mat4(1.0f), m_scale) * resultRotation * PrismaMatrix;
    PrismaMatrix[3] = PrismaMatrix[3] - scalingVector;
    PrismaMatrix[3][3] = 1;

    glm::vec3 extents = (meshData.mesh->aabbData().max - meshData.mesh->aabbData().min) * 0.5f;
    PrismaMatrix = glm::translate(PrismaMatrix, glm::vec3(-meshData.mesh->aabbData().min.x - extents.x, -meshData.mesh->aabbData().min.y - extents.y, -meshData.mesh->aabbData().min.z - extents.z));
    
    m_currentModel = Prisma::createModelMatrix(m_translation, finalRotation, m_scale);
    
    meshData.mesh->matrix(m_currentModel);

    meshData.mesh->finalMatrix(PrismaMatrix);

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


        glm::vec3 eulerRotation = glm::degrees(glm::eulerAngles(m_rotation));
        glm::vec3 eulerRotation1 = glm::radians(eulerRotation);
        glm::mat4 finalRotation=glm::rotate(glm::mat4(1.0f),eulerRotation1.x,glm::vec3(1.0f,0.0f,0.0f))*glm::rotate(glm::mat4(1.0f),eulerRotation1.y,glm::vec3(0.0f,1.0f,0.0f))*glm::rotate(glm::mat4(1.0f),eulerRotation1.z,glm::vec3(0.0f,0.0f,1.0f));

        if(ImGui::InputFloat3("Translation",glm::value_ptr(m_translation))){
            m_currentModel=Prisma::createModelMatrix(m_translation,finalRotation,m_scale);
        }

        if(ImGui::InputFloat3("Rotation",glm::value_ptr(eulerRotation))){
            m_rotation=glm::quat(glm::radians(eulerRotation));
            eulerRotation1 = glm::radians(eulerRotation);
            finalRotation=glm::rotate(glm::mat4(1.0f),eulerRotation1.x,glm::vec3(1.0f,0.0f,0.0f))*glm::rotate(glm::mat4(1.0f),eulerRotation1.y,glm::vec3(0.0f,1.0f,0.0f))*glm::rotate(glm::mat4(1.0f),eulerRotation1.z,glm::vec3(0.0f,0.0f,1.0f));
            //rotation=Prisma::eulerToQuaternion(eulerRotation.x,eulerRotation.y,eulerRotation.z);
            m_currentModel=Prisma::createModelMatrix(m_translation,finalRotation,m_scale);
        }


        if(ImGui::InputFloat3("Scale",glm::value_ptr(m_scale))){
            m_currentModel=Prisma::createModelMatrix(m_translation,finalRotation,m_scale);
        }

        if(ImGui::Button("Apply")){

            glm::vec3 halfExtents = (meshData.mesh->aabbData().max - meshData.mesh->aabbData().min)*0.5f;
            glm::vec3 origin(meshData.mesh->aabbData().min.x + halfExtents.x, meshData.mesh->aabbData().min.y + halfExtents.y,
                             meshData.mesh->aabbData().min.z + halfExtents.z);
            glm::mat4 PrismaMatrix=glm::translate(glm::mat4(1.0f),m_translation+origin);

            auto scalingVector=PrismaMatrix[3]*(glm::vec4(m_scale-glm::vec3(1.0f),1.0));

            glm::mat4 trans_to_pivot   = glm::translate(glm::mat4(1.0f), -(m_translation+origin));
            glm::mat4 trans_from_pivot = glm::translate(glm::mat4(1.0f), m_translation+origin);
            glm::mat4 rotate_matrix=finalRotation;

            glm::mat4 resultRotation=trans_from_pivot * rotate_matrix * trans_to_pivot;

            PrismaMatrix=glm::scale(glm::mat4(1.0f),m_scale)*resultRotation*PrismaMatrix;
            PrismaMatrix[3]=PrismaMatrix[3]-scalingVector;
            PrismaMatrix[3][3]=1;

            glm::vec3 extents = (meshData.mesh->aabbData().max - meshData.mesh->aabbData().min)*0.5f;
            PrismaMatrix=glm::translate(PrismaMatrix,glm::vec3(-meshData.mesh->aabbData().min.x-extents.x,-meshData.mesh->aabbData().min.y-extents.y,-meshData.mesh->aabbData().min.z-extents.z));
            meshData.mesh->matrix(m_currentModel);

            meshData.mesh->finalMatrix(PrismaMatrix);
            
            skipUpdate=true;
        }

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
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m_currentModel, m_scale, m_rotation, m_translation, skew, perspective);
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
