#include <iostream>
#include "../include/SettingsTab.h"
#include "imgui.h"
#include "../../Engine/include/GlobalData/PrismaFunc.h"

void Prisma::SettingsTab::init()
{
	if (m_settings.msaa) {
		Prisma::PrismaFunc::getInstance().msaa(m_settings.msaa, m_settings.numSamples);
	}
}

void Prisma::SettingsTab::drawSettings()
{
    bool closed = true;
    if (ImGui::BeginPopupModal("SettingsTab",&closed)) {
        if (!m_position) {
            ImGui::SetWindowPos(ImVec2(0, 100));
            m_position = true;
        }
        if (ImGui::Checkbox("MSAA",&m_settings.msaa))
        {
            Prisma::PrismaFunc::getInstance().msaa(m_settings.msaa, m_settings.numSamples);
        }
        if (!closed)
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
