#include "../include/LightInfo.h"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "GlobalData/CacheScene.h"
#include "../include/ImGuiStyle.h"


void Prisma::GUI::LightInfo::showSelectedDir(std::shared_ptr<Light<LightType::LightDir>> lightData,
                                             const NodeViewer::NodeData& meshData) {
    auto type = lightData->type();
    float windowWidth = meshData.translate * meshData.width / 2.0f;
    auto nextRight = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
    };
    nextRight(meshData.initOffset);
    ImGui::Begin(lightData->name().c_str(), nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGuiStyles::getInstance().treeStyle();
    if (ImGui::CollapsingHeader("Directional data")) {
        ImGuiStyles::getInstance().clearTreeStyle();
        if (ImGui::ColorPicker3("Diffuse ", value_ptr(type.diffuse))) {
            lightData->type(type);
        }

        float intensity = lightData->intensity();

        if (ImGui::InputFloat("Intensity ", &intensity)) {
            lightData->intensity(intensity);
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

            auto csmShadow = std::dynamic_pointer_cast<PipelineCSM>(shadow);

            float bias = csmShadow->bias();

            if (ImGui::InputFloat("Far Plane ", &farPlane)) {
                shadow->farPlane(farPlane);
            }

            if (ImGui::InputFloat("Near Plane ", &nearPlane)) {
                shadow->nearPlane(nearPlane);
            }

            if (ImGui::InputFloat("Bias ", &bias)) {
                csmShadow->bias(bias);
            }
        }
    } else {
        ImGuiStyles::getInstance().clearTreeStyle();
    }

    ImGui::End();
}

void Prisma::GUI::LightInfo::showSelectedOmni(std::shared_ptr<Light<LightType::LightOmni>> lightData,
                                              const NodeViewer::NodeData& meshData) {
    auto type = lightData->type();
    float windowWidth = meshData.translate * meshData.width / 2.0f;
    auto nextRight = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
    };
    nextRight(meshData.initOffset);
    ImGui::Begin(lightData->name().c_str(), nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGuiStyles::getInstance().treeStyle();
    if (ImGui::CollapsingHeader("Omnidirectional data")) {
        ImGuiStyles::getInstance().clearTreeStyle();
        if (ImGui::ColorPicker3("Diffuse ", value_ptr(type.diffuse))) {
            lightData->type(type);
        }

        float intensity = lightData->intensity();

        if (ImGui::InputFloat("Intensity ", &intensity)) {
            lightData->intensity(intensity);
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
    } else {
        ImGuiStyles::getInstance().clearTreeStyle();
    }

    ImGui::End();
}

void Prisma::GUI::LightInfo::showSelectedArea(std::shared_ptr<Light<LightType::LightArea>> lightData,
                                              const NodeViewer::NodeData& meshData) {
    auto type = lightData->type();
    float windowWidth = meshData.translate * meshData.width / 2.0f;
    auto nextRight = [&](float pos) {
        ImGui::SetNextWindowPos(ImVec2(windowWidth + meshData.scale * meshData.width, pos));
        ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));
    };
    nextRight(meshData.initOffset);
    ImGui::Begin(lightData->name().c_str(), nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGuiStyles::getInstance().treeStyle();
    if (ImGui::CollapsingHeader("Area data")) {
        ImGuiStyles::getInstance().clearTreeStyle();
        if (ImGui::ColorPicker3("Diffuse ", value_ptr(type.diffuse))) {
            lightData->type(type);
            if (meshData.node && meshData.node->parent()) {
                for (auto mesh : meshData.node->parent()->children()) {
                    auto isMesh = std::dynamic_pointer_cast<Mesh>(mesh);
                    if (isMesh) {
                        auto material = isMesh->material();
                        material->color(type.diffuse);
                        isMesh->material(material);
                    }
                }
            }
        }

        float intensity = lightData->intensity();

        if (ImGui::InputFloat("Intensity ", &intensity)) {
            lightData->intensity(intensity);
        }
        bool doubleSide = type.doubleSide;
        if (ImGui::Checkbox("Double side", &doubleSide)) {
            type.doubleSide = doubleSide;
            lightData->type(type);
        }
    } else {
        ImGuiStyles::getInstance().clearTreeStyle();
    }

    ImGui::End();
}

glm::vec3 Prisma::GUI::LightInfo::directionToEulerAngles(const glm::vec3& direction) {
    // Compute yaw (heading) angle
    float yaw = atan2(direction.x, direction.z);

    // Compute pitch (elevation) angle
    float pitch = asin(direction.y);

    // Since we only have the direction vector and no roll information, roll is set to 0
    float roll = 0.0f;

    return glm::vec3(pitch, yaw, roll);
}