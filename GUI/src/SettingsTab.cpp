#include "../include/SettingsTab.h"
#include "imgui.h"
#include "../../Engine/include/GlobalData/PrismaFunc.h"

void Prisma::SettingsTab::init()
{
	if (m_settings.msaa) {
		Prisma::PrismaFunc::getInstance().msaa(m_settings.msaa, m_settings.numSamples);
	}
}

void Prisma::SettingsTab::drawSettings(bool& show)
{
	ImGui::Begin("Dummy Top", &show);
	if (ImGui::Checkbox("MSAA",&m_settings.msaa))
	{
		Prisma::PrismaFunc::getInstance().msaa(m_settings.msaa, m_settings.numSamples);
	}
	ImGui::End();
}
