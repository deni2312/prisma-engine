#include "../include/LightInfo.h"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "../../Engine/include/GlobalData/CacheScene.h"


void Prisma::LightInfo::showSelectedDir(Prisma::Light<Prisma::LightType::LightDir>* lightData,const Prisma::MeshInfo::MeshData& meshData)
{
    auto type = lightData->type();
    float windowWidth = meshData.translate * meshData.width / 2.0f;
    auto nextRight = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
    };
    nextRight(meshData.initOffset);
    ImGui::Begin(lightData->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // Decompose the matrix
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;

    glm::mat4 model = lightData->parent()->matrix();
    glm::mat4 inverseParent = glm::inverse(lightData->parent()->parent()->finalMatrix());
    ImGuiIO& io = ImGui::GetIO();
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(glm::value_ptr(meshData.camera->matrix()), glm::value_ptr(meshData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));
    
    lightData->parent()->matrix(inverseParent * model);


    if (ImGuizmo::IsUsing()) {
    }

    // Convert quaternion to Euler angles (XYZ)
    glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));


    ImGui::InputFloat3("Rotation ", glm::value_ptr(euler));

    if (ImGui::InputFloat3("Diffuse ", glm::value_ptr(type.diffuse))) {
        lightData->type(type);
    }

    bool hasShadow = lightData->hasShadow();

    if (ImGui::Checkbox("Shadow ", &hasShadow)) {
        lightData->hasShadow(hasShadow);
    }

    auto shadow = lightData->shadow();

    if (hasShadow && shadow) {
        auto shadow = lightData->shadow();

        float farPlane = shadow->farPlane();
        float nearPlane = shadow->nearPlane();

        auto csmShadow = std::dynamic_pointer_cast<Prisma::PipelineCSM>(shadow);

        float zMult = csmShadow->zMult();

        if (ImGui::InputFloat("Far Plane ", &farPlane)) {
            shadow->farPlane(farPlane);
        }

        if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
            shadow->nearPlane(nearPlane);
        }

        if (ImGui::InputFloat("ZMult ", &zMult)) {
            csmShadow->zMult(zMult);
        }
    }


    ImGui::End();
}

void Prisma::LightInfo::showSelectedOmni(Prisma::Light<Prisma::LightType::LightOmni>* lightData,const Prisma::MeshInfo::MeshData& meshData)
{
    auto type = lightData->type();
    float windowWidth = meshData.translate * meshData.width / 2.0f;
    auto nextRight = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
    };
    nextRight(meshData.initOffset);
    ImGui::Begin(lightData->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;

    glm::mat4 model = lightData->parent()->matrix();
    glm::mat4 inverseParent = glm::inverse(lightData->parent()->parent()->finalMatrix());
    ImGuiIO& io = ImGui::GetIO();
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(glm::value_ptr(meshData.camera->matrix()), glm::value_ptr(meshData.projection), mCurrentGizmoOperation, mCurrentGizmoMode, glm::value_ptr(model));

    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

    lightData->parent()->matrix(inverseParent * model);

    ImGui::InputFloat3("Translation ", glm::value_ptr(type.position));

    if (ImGui::InputFloat3("Diffuse ", glm::value_ptr(type.diffuse))) {
        lightData->type(type);
    }

    if (ImGui::InputFloat("Radius ", &type.radius)) {
        lightData->type(type);
    }

    bool hasShadow = lightData->hasShadow();

    if (ImGui::Checkbox("Shadow ", &hasShadow)) {
        lightData->hasShadow(hasShadow);
    }

    auto shadow = lightData->shadow();

    if (hasShadow && shadow) {

        float farPlane = shadow->farPlane();
        float nearPlane = shadow->nearPlane();

        if (ImGui::InputFloat("Far Plane ", &farPlane)) {
            shadow->farPlane(farPlane);
        }

        if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
            shadow->nearPlane(nearPlane);
        }

    }

    ImGui::End();
}

glm::vec3 Prisma::LightInfo::directionToEulerAngles(const glm::vec3& direction) {
    // Compute yaw (heading) angle
    float yaw = atan2(direction.x, direction.z);

    // Compute pitch (elevation) angle
    float pitch = asin(direction.y);

    // Since we only have the direction vector and no roll information, roll is set to 0
    float roll = 0.0f;

    return glm::vec3(pitch, yaw, roll);
}
