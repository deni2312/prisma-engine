#include <iostream>
#include <iostream>

#include "../include/SettingsTab.h"
#include "GlobalData/PrismaFunc.h"
#include "GlobalData/CacheScene.h"
#include "engine.h"
#include "Postprocess/Postprocess.h"
#include "../include/ImGuiDebug.h"
#include "../include/TextureInfo.h"
#include <ThirdParty/imgui/imgui.h>

#include "Handlers/TLASHandler.h"

#include "Pipelines/PipelineHandler.h"

Prisma::GUI::ImguiDebug::ImGuiStatus m_status;

void Prisma::GUI::SettingsTab::init() {
    m_effects = std::make_shared<Effects>();
    m_fxaa = std::make_shared<FXAA>();
    Postprocess::getInstance().addPostProcess(m_fxaa);
    Postprocess::getInstance().addPostProcess(m_effects);
    m_status.currentitem = static_cast<unsigned int>(Engine::getInstance().engineSettings().pipeline);
    m_status.currentPostprocess = 0;

    m_status.items.push_back("FORWARD");
    m_status.items.push_back("DEFERRED FORWARD");
    m_status.items.push_back("RAYTRACING");
    m_status.items.push_back("SOFTWARE RAYTRACING");

    m_status.postprocess.push_back("NORMAL");
    m_status.postprocess.push_back("SEPPIA");
    m_status.postprocess.push_back("CARTOON");
    m_status.postprocess.push_back("VIGNETTE");
    m_status.postprocess.push_back("BLOOM");
    m_status.postprocess.push_back("VOLUMETRIC");
    m_status.postprocess.push_back("VOLUMETRIC RAYS");
}

void Prisma::GUI::SettingsTab::drawSettings() {
    bool closed = true;

    if (ImGui::BeginPopupModal("SettingsTab", &closed)) {
        if (!m_position) {
            ImGui::SetWindowPos(ImVec2(0, 100));
            ImGui::SetWindowSize(ImVec2(600, 400));
            m_position = true;
        }
        ImGui::Text("CACHE STATUS:");
        auto stringBool = [](bool data) { return data ? std::string("TRUE") : std::string("FALSE"); };
        ImGui::Text(("    MODEL: " + stringBool(CacheScene::getInstance().updateData())).c_str());
        ImGui::Text(("    LIGHT: " + stringBool(CacheScene::getInstance().updateLights())).c_str());
        ImGui::Text(("    SIZE: " + stringBool(CacheScene::getInstance().updateSizes())).c_str());
        ImGui::Text(("    TEXTURE: " + stringBool(CacheScene::getInstance().updateTextures())).c_str());
        ImGui::Text(("    STATUS: " + stringBool(CacheScene::getInstance().updateStatus())).c_str());

        if (ImGui::Combo("PIPELINE", &m_status.currentitem, m_status.items.data(), m_status.items.size())) {
            switch (static_cast<EngineSettings::Pipeline>(m_status.currentitem)) {
                case EngineSettings::Pipeline::FORWARD:
                    PipelineHandler::getInstance().forward();
                    break;
                case EngineSettings::Pipeline::DEFERRED_FORWARD:
                    PipelineHandler::getInstance().deferredForward();
                    break;
                case EngineSettings::Pipeline::SOFTWARE_RAYTRACING:
                    PipelineHandler::getInstance().softwareRt()->loadData();
                    break;
                case EngineSettings::Pipeline::RAYTRACING:
                    PipelineHandler::getInstance().raytracing();
                    TLASHandler::getInstance().updateSizeTLAS();

                    int maxHardware = PipelineHandler::getInstance().raytracing()->
                                                                     hardwareMaxReflection();

                    int maxRecursionDepth = PipelineHandler::getInstance().raytracing()->
                                                                           maxRecursion();

                    ImGui::SliderInt("Max Recursion Depth##1", &maxRecursionDepth, 1, maxHardware);

                    int maxRecursionReflection = PipelineHandler::getInstance().raytracing()->
                                                                                maxRecursionReflection();

                    ImGui::SliderInt("Max Recursion Reflection##1", &maxRecursionReflection, 1,
                                     maxHardware);
                    break;
            }
            CacheScene::getInstance().updateSizes(true);
        }

        ImGui::Combo("POSTPROCESS", &m_status.currentPostprocess, m_status.postprocess.data(),
                     m_status.postprocess.size());
        
        bool isFxaa = m_fxaa->apply();

        ImGui::Checkbox("FXAA", &isFxaa);

        m_fxaa->apply(isFxaa);

        auto settings = Engine::getInstance().engineSettings();

        ImGui::Checkbox("SSAO", &settings.ssao);

        ImGui::Checkbox("SCREEN SPACE REFLECTIONS", &settings.ssr);

        //ImGui::Checkbox("SCREEN SPACE AMBIENT OCCLUSION", &settings.ssao);
        
        bool debugPhysics = Physics::getInstance().debug();

        ImGui::Checkbox("PHYSICS DEBUG", &debugPhysics);
        Physics::getInstance().debug(debugPhysics);

        //
        //bool sortTransparencies = Prisma::GlobalData::getInstance().transparencies();
        //
        //ImGui::Checkbox("SORT TRANSPARENCIES", &sortTransparencies);

        float scale = ImguiDebug::getInstance().scale();

        ImGui::SliderFloat("Scale##1", &scale, 0.1, 1);

        switch (static_cast<EngineSettings::Pipeline>(m_status.currentitem)) {
            case EngineSettings::Pipeline::FORWARD:
                break;
            case EngineSettings::Pipeline::RAYTRACING:

                int maxHardware = PipelineHandler::getInstance().raytracing()->hardwareMaxReflection();

                int maxRecursionDepth = PipelineHandler::getInstance().raytracing()->maxRecursion();

                ImGui::SliderInt("Max Recursion Depth##1", &maxRecursionDepth, 1, maxHardware);

                int maxRecursionReflection = PipelineHandler::getInstance().raytracing()->
                                                                            maxRecursionReflection();

                ImGui::SliderInt("Max Recursion Reflection##1", &maxRecursionReflection, 1,
                                 maxHardware);

                bool raytracingEasy = PipelineHandler::getInstance().raytracing()->raytracingEasy();

                ImGui::Checkbox("Raytracing easy##1", &raytracingEasy);

                PipelineHandler::getInstance().raytracing()->raytracingEasy(raytracingEasy);

                PipelineHandler::getInstance().raytracing()->maxRecursion(maxRecursionDepth);
                PipelineHandler::getInstance().raytracing()->maxRecursionReflection(
                    maxRecursionReflection);
                break;
        }

        switch (static_cast<PostprocessingStyles::EFFECTS>(m_status.currentPostprocess)) {
            case PostprocessingStyles::EFFECTS::VOLUMETRIC: {
                auto volumetric = m_effects->volumetricRender();
                auto volumetricSettings = volumetric->volumetricSettings();
                ImGui::ColorEdit4("Fog Color", glm::value_ptr(volumetricSettings.fogColor));
                ImGui::SliderFloat("Max Distance", &volumetricSettings.maxDistance.r, 1.0f, 500.0f);
                ImGui::SliderFloat("Step Size", &volumetricSettings.stepSize.r, 0.1f, 20.0f);
                ImGui::SliderFloat("Density Multiplier", &volumetricSettings.densityMultiplier.r, 0.0f, 1.0f);
                ImGui::SliderFloat("Noise Offset", &volumetricSettings.noiseOffset.r, 0.0f, 10.0f);
                ImGui::SliderFloat("Density Threshold", &volumetricSettings.densityThreshold.r, 0.0f, 1.0f);
                ImGui::SliderFloat("Noise Tiling", &volumetricSettings.noiseTiling.r, 0.1f, 10.0f);
                ImGui::ColorEdit4("Light Contribution", glm::value_ptr(volumetricSettings.lightContribution));
                ImGui::SliderFloat("Light Scattering", &volumetricSettings.lightScattering.r, 0.0f, 1.0f);
                volumetric->volumetricSettings(volumetricSettings);
                break;
            }
            case PostprocessingStyles::EFFECTS::RAYS: {
                auto volumetric = m_effects->volumetricRaysRender();
                auto volumetricSettings = volumetric->volumetricSettings();
                ImGui::SliderFloat("Density", &volumetricSettings.density.r,0,1);
                ImGui::SliderFloat("Decay", &volumetricSettings.decay.r, 0, 1);
                ImGui::SliderFloat("Exposure", &volumetricSettings.exposure.r, 0, 1);
                ImGui::SliderFloat("Weight", &volumetricSettings.weight.r, 0, 1);
                ImGui::SliderInt("Samples", &volumetricSettings.samples.r, 0, 100);

                volumetric->volumetricSettings(volumetricSettings);
                break;
            }
        }

        //Prisma::GlobalData::getInstance().transparencies(sortTransparencies);

        Engine::getInstance().engineSettings(settings);

        ImguiDebug::getInstance().scale(scale);

        if (!closed)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Prisma::GUI::SettingsTab::updateStatus() {
    auto engineSettings = Engine::getInstance().engineSettings();
    engineSettings.pipeline = static_cast<EngineSettings::Pipeline>(m_status.currentitem);
    Engine::getInstance().engineSettings(engineSettings);
    m_effects->effect(static_cast<PostprocessingStyles::EFFECTS>(m_status.currentPostprocess));
}