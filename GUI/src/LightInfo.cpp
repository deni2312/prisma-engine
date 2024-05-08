#include "../include/LightInfo.h"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>


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
    glm::vec3 scale, skew;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec4 perspective;
    glm::decompose(lightData->parent()->finalMatrix(), scale, rotation, translation, skew, perspective);

    // Convert quaternion to Euler angles (XYZ)
    glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));


    if (ImGui::InputFloat3("Rotation ", glm::value_ptr(euler))) {
        euler = glm::radians(euler);
        glm::mat4 rotationMatrix(1.0f); // Identity matrix

        // Rotate around Z axis
        rotationMatrix = glm::rotate(rotationMatrix, euler.z, glm::vec3(0.0f, 0.0f, 1.0f));

        // Rotate around Y axis
        rotationMatrix = glm::rotate(rotationMatrix, euler.y, glm::vec3(0.0f, 1.0f, 0.0f));

        // Rotate around X axis
        rotationMatrix = glm::rotate(rotationMatrix, euler.x, glm::vec3(1.0f, 0.0f, 0.0f));

        lightData->parent()->matrix(rotationMatrix);
        skipUpdate = true;
    }

    if (ImGui::InputFloat3("Diffuse ", glm::value_ptr(type.diffuse))) {
        lightData->type(type);
        skipUpdate = true;
    }

    bool hasShadow = lightData->hasShadow();

    if (ImGui::Checkbox("Shadow ", &hasShadow)) {
        lightData->hasShadow(hasShadow);
        skipUpdate = true;
    }

    auto shadow = lightData->shadow();

    if (hasShadow && shadow) {
        auto shadow = lightData->shadow();

        float farPlane = shadow->farPlane();
        float nearPlane = shadow->nearPlane();

        auto csmShadow = std::dynamic_pointer_cast<Prisma::PipelineCSM>(shadow);

        if (ImGui::InputFloat("Far Plane ", &farPlane)) {
            shadow->farPlane(farPlane);
            skipUpdate = true;
        }

        if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
            shadow->nearPlane(nearPlane);
            skipUpdate = true;
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
    if (ImGui::InputFloat3("Translation ", glm::value_ptr(type.position))) {
        lightData->type(type);
        skipUpdate = true;
    }

    if (ImGui::InputFloat3("Diffuse ", glm::value_ptr(type.diffuse))) {
        lightData->type(type);
        skipUpdate = true;
    }

    if (ImGui::InputFloat("Radius ", &type.radius)) {
        lightData->type(type);
        skipUpdate = true;
    }

    bool hasShadow = lightData->hasShadow();

    if (ImGui::Checkbox("Shadow ", &hasShadow)) {
        lightData->hasShadow(hasShadow);
        skipUpdate = true;
    }

    auto shadow = lightData->shadow();

    if (hasShadow && shadow) {

        float farPlane = shadow->farPlane();
        float nearPlane = shadow->nearPlane();

        if (ImGui::InputFloat("Far Plane ", &farPlane)) {
            shadow->farPlane(farPlane);
            skipUpdate = true;
        }

        if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
            shadow->nearPlane(nearPlane);
            skipUpdate = true;
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
