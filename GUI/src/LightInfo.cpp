#include "../include/LightInfo.h"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/string_cast.hpp>


void Prisma::LightInfo::showSelectedDir(Prisma::Light<Prisma::LightType::LightDir>* lightData, Prisma::MeshInfo::MeshData meshData)
{
    auto type = lightData->type();
    float windowWidth = meshData.translate * meshData.width / 2.0f;
    auto nextRight = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
    };
    nextRight(meshData.initOffset);
    ImGui::Begin(lightData->name().c_str(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    glm::vec3 eulerRotation = glm::degrees(type.direction);

    if (ImGui::InputFloat3("Rotation ", glm::value_ptr(eulerRotation))) {
        type.direction = glm::vec4(glm::radians(eulerRotation),1.0f);
        lightData->type(type);
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

    if (hasShadow) {
        auto shadow = lightData->shadow();

        float farPlane = shadow->farPlane();
        float nearPlane = shadow->nearPlane();

        if (ImGui::InputFloat("Far Plane ", &farPlane)) {
            shadow->farPlane(farPlane);
        }

        if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
            shadow->nearPlane(nearPlane);
        }

        skipUpdate = true;
    }


    ImGui::End();
}

void Prisma::LightInfo::showSelectedOmni(Prisma::Light<Prisma::LightType::LightOmni>* lightData, Prisma::MeshInfo::MeshData meshData)
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

    if (hasShadow) {
        auto shadow = lightData->shadow();

        float farPlane = shadow->farPlane();
        float nearPlane = shadow->nearPlane();

        if (ImGui::InputFloat("Far Plane ", &farPlane)) {
            shadow->farPlane(farPlane);
        }

        if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
            shadow->nearPlane(nearPlane);
        }

        skipUpdate = true;
    }

    ImGui::End();
}
