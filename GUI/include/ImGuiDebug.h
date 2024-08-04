#pragma once
#include "Postprocess/Effects.h"
#include "ImGuiCamera.h"
#include "LightInfo.h"
#include "ImGuiTabs.h"
#include "MeshInfo.h"
#include "FolderView.h"
#include "SettingsTab.h"
#include <memory>
#include <vector>
#include <string>

namespace Prisma {
	class ImguiDebug {
	public:
		struct ImGuiData {
			std::vector<std::shared_ptr<std::pair<std::string, float>>> performances;
		};



        struct ImGuiStatus {
            //PIPELINE DATA
            std::vector<const char*> items;
            int currentitem = 0;

			std::vector<const char*> postprocess;
			int currentPostprocess = 0;
        };
		ImguiDebug();
		void drawGui();
		float fps();
		void start();
		void close();
		void imguiData(std::shared_ptr<ImGuiData> data);
		std::shared_ptr<Prisma::FBO> fbo();

		static ImguiDebug& getInstance();
		ImguiDebug(const ImguiDebug&) = delete;
		ImguiDebug& operator=(const ImguiDebug&) = delete;

		std::shared_ptr<SceneHandler> handlers();

	private:
		static std::shared_ptr<ImguiDebug> instance;
		double m_lastFrameTime = 0.0;
		float m_fps;
		std::shared_ptr<ImGuiData> m_data;
		std::shared_ptr<Prisma::FBO> m_fbo;
		std::shared_ptr<Shader> m_shader;
		unsigned int m_bindlessPos;
		unsigned int m_modelPos;
		glm::mat4 m_model;

		float m_scale;
		float m_translate;
		unsigned int m_width;
		unsigned int m_height;

		void drawScene();
		void initStatus();
		std::string openFolder();
		std::string saveFile();


		float m_initOffset = 0.0f;

		ImGuiCamera m_imguiCamera;

		std::shared_ptr<Prisma::Camera> m_camera;

        Prisma::MeshInfo meshInfo;

		Prisma::LightInfo lightInfo;

        std::shared_ptr<Prisma::FileBrowser> m_fileBrowser;

		glm::mat4 m_projection;

		bool m_run = false;

		std::string m_saveLocation;

		std::shared_ptr<Prisma::Texture> m_runButton;

		std::shared_ptr<Prisma::Texture> m_pauseButton;

		SettingsTab m_settingsTab;

		bool m_settingsTabShow = false;

    };
}