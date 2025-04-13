#include <iostream>
#include "../include/SettingsTab.h"
#include "GlobalData/PrismaFunc.h"
#include "GlobalData/CacheScene.h"
#include "engine.h"
#include "Postprocess/Postprocess.h"
#include "../include/ImGuiDebug.h"
#include "../include/TextureInfo.h"
#include <ThirdParty/imgui/imgui.h>

#include "Helpers/UpdateTLAS.h"
#include "Pipelines/PipelineHandler.h"

Prisma::ImguiDebug::ImGuiStatus m_status;

void Prisma::SettingsTab::init()
{
	/*m_effects = std::make_shared<Effects>();
	m_effectsBloom = std::make_shared<Effects>();
	Postprocess::getInstance().addPostProcess(m_effectsBloom);
	Postprocess::getInstance().addPostProcess(m_effects);*/
	m_status.currentitem = static_cast<unsigned int>(Prisma::Engine::getInstance().engineSettings().pipeline);
	m_status.currentPostprocess = 0;

	m_status.items.push_back("FORWARD");
	m_status.items.push_back("DEFERRED");
	m_status.items.push_back("DEFERRED_FORWARD");
	m_status.items.push_back("RAYTRACING");

	m_status.postprocess.push_back("NORMAL");
	m_status.postprocess.push_back("SEPPIA");
	m_status.postprocess.push_back("CARTOON");
	m_status.postprocess.push_back("VIGNETTE");
}

void Prisma::SettingsTab::drawSettings()
{
	bool closed = true;

	if (ImGui::BeginPopupModal("SettingsTab", &closed))
	{
		if (!m_position)
		{
			ImGui::SetWindowPos(ImVec2(0, 100));
			m_position = true;
		}
		ImGui::Text("CACHE STATUS:");
		auto stringBool = [](bool data) { return data ? std::string("TRUE") : std::string("FALSE"); };
		ImGui::Text(("    MODEL: " + stringBool(CacheScene::getInstance().updateData())).c_str());
		ImGui::Text(("    LIGHT: " + stringBool(CacheScene::getInstance().updateLights())).c_str());
		ImGui::Text(("    SIZE: " + stringBool(CacheScene::getInstance().updateSizes())).c_str());
		ImGui::Text(("    TEXTURE: " + stringBool(CacheScene::getInstance().updateTextures())).c_str());
		ImGui::Text(("    STATUS: " + stringBool(CacheScene::getInstance().updateStatus())).c_str());

		if (ImGui::Combo("PIPELINE", &m_status.currentitem, m_status.items.data(), m_status.items.size()))
		{
			switch (m_status.currentitem)
			{
			case EngineSettings::Pipeline::FORWARD:
				Prisma::PipelineHandler::getInstance().forward();
				break;
			case EngineSettings::Pipeline::DEFERRED:
				break;
			case EngineSettings::Pipeline::DEFERRED_FORWARD:
				break;
			case EngineSettings::Pipeline::RAYTRACING:
				Prisma::PipelineHandler::getInstance().raytracing();
				Prisma::UpdateTLAS::getInstance().updateSizeTLAS();
				break;
			}
			Prisma::CacheScene::getInstance().updateSizes(true);
		}
		ImGui::Combo("POSTPROCESS", &m_status.currentPostprocess, m_status.postprocess.data(),
		             m_status.postprocess.size());
		ImGui::Checkbox("BLOOM", &m_bloom);

		auto settings = Engine::getInstance().engineSettings();

		ImGui::Checkbox("SCREEN SPACE REFLECTIONS", &settings.ssr);

		ImGui::Checkbox("SCREEN SPACE AMBIENT OCCLUSION", &settings.ssao);

		bool debugPhysics = Physics::getInstance().debug();

		ImGui::Checkbox("PHYSICS DEBUG", &debugPhysics);

		bool sortTransparencies = Prisma::GlobalData::getInstance().transparencies();

		ImGui::Checkbox("SORT TRANSPARENCIES", &sortTransparencies);

		float scale = Prisma::ImguiDebug::getInstance().scale();

		ImGui::SliderFloat("Scale##1", &scale, 0.1, 1);

		Prisma::GlobalData::getInstance().transparencies(sortTransparencies);

		Physics::getInstance().debug(debugPhysics);

		Engine::getInstance().engineSettings(settings);

		Prisma::ImguiDebug::getInstance().scale(scale);

		if (!closed)
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}

void Prisma::SettingsTab::updateStatus()
{
	auto engineSettings = Engine::getInstance().engineSettings();
	engineSettings.pipeline = static_cast<EngineSettings::Pipeline>(m_status.currentitem);
	Engine::getInstance().engineSettings(engineSettings);

	/*m_effects->effect(static_cast<Effects::EFFECTS>(m_status.currentPostprocess));

	if (m_bloom)
	{
		m_effectsBloom->effect(Effects::EFFECTS::BLOOM);
	}
	else
	{
		m_effectsBloom->effect(Effects::EFFECTS::NORMAL);
	}*/
}
