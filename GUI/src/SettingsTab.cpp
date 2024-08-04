#include <iostream>
#include "../include/SettingsTab.h"
#include "imgui.h"
#include "../../Engine/include/GlobalData/PrismaFunc.h"
#include "../../Engine/include/GlobalData/CacheScene.h"
#include "../../Engine/include/engine.h"
#include "../../Engine/include/Postprocess/Postprocess.h"
#include "../include/ImGuiDebug.h"
#include "../include/TextureInfo.h"

Prisma::ImguiDebug::ImGuiStatus m_status;

void Prisma::SettingsTab::init()
{
    m_effects = std::make_shared<Prisma::Effects>();
    m_effectsBloom = std::make_shared<Prisma::Effects>();
    Prisma::Postprocess::getInstance().addPostProcess(m_effectsBloom);
    Prisma::Postprocess::getInstance().addPostProcess(m_effects);
    m_status.currentitem = 0;
    m_status.currentPostprocess = 0;

    m_status.items.push_back("FORWARD");
    m_status.items.push_back("DEFERRED");

    m_status.postprocess.push_back("NORMAL");
    m_status.postprocess.push_back("SEPPIA");
    m_status.postprocess.push_back("CARTOON");
    m_status.postprocess.push_back("VIGNETTE");
}

void Prisma::SettingsTab::drawSettings()
{
    bool closed = true;

    if (ImGui::BeginPopupModal("SettingsTab",&closed)) {
        if (!m_position) {
            ImGui::SetWindowPos(ImVec2(0, 100));
            m_position = true;
        }
        ImGui::Text("CACHE STATUS:");
        auto stringBool = [](bool data) {return data ? std::string("TRUE") : std::string("FALSE"); };
        ImGui::Text(("    MODEL: " + stringBool(Prisma::CacheScene::getInstance().updateData())).c_str());
        ImGui::Text(("    LIGHT: " + stringBool(Prisma::CacheScene::getInstance().updateLights())).c_str());
        ImGui::Text(("    SIZE: " + stringBool(Prisma::CacheScene::getInstance().updateSizes())).c_str());
        ImGui::Text(("    TEXTURE: " + stringBool(Prisma::CacheScene::getInstance().updateTextures())).c_str());
        ImGui::Text(("    STATUS: " + stringBool(Prisma::CacheScene::getInstance().updateStatus())).c_str());

        ImGui::Combo("PIPELINE", &m_status.currentitem, m_status.items.data(), m_status.items.size());
        ImGui::Combo("POSTPROCESS", &m_status.currentPostprocess, m_status.postprocess.data(), m_status.postprocess.size());
        ImGui::Checkbox("BLOOM", &m_bloom);

        if (!closed)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Prisma::SettingsTab::updateStatus()
{
    Prisma::Engine::getInstance().pipeline(static_cast<Prisma::Engine::Pipeline>(m_status.currentitem));

    m_effects->effect(static_cast<Prisma::Effects::EFFECTS>(m_status.currentPostprocess));

    if (m_bloom) {
        m_effectsBloom->effect(Prisma::Effects::EFFECTS::BLOOM);
    }
    else {
        m_effectsBloom->effect(Prisma::Effects::EFFECTS::NORMAL);
    }
}
