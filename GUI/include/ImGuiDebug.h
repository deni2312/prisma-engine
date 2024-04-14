#pragma once
#include "../../Engine/include/Containers/FBO.h"
#include "../../Engine/include/Helpers/Shader.h"
#include "../../Engine/include/engine.h"
#include "../../Engine/include/Helpers/Settings.h"
#include "Postprocess/Effects.h"
#include "ImGuiCamera.h"
#include "LightInfo.h"
#include "ImGuiTabs.h"
#include "GLFW/glfw3.h"
#include "MeshInfo.h"
#include "FolderView.h"
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
		ImguiDebug(GLFWwindow* window, const unsigned int& width, const unsigned int& height, void* engine);
		void drawGui();
		float fps();
		void start();
		void close();
		void imguiData(std::shared_ptr<ImGuiData> data);
		void updateStatus();
		std::shared_ptr<Prisma::FBO> fbo();
	private:
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

		float m_initOffset = 0.0f;

		ImGuiCamera m_imguiCamera;

		std::shared_ptr<Prisma::Camera> m_camera;

		ImGuiStatus m_status;

        Prisma::MeshInfo meshInfo;

		Prisma::LightInfo lightInfo;

        std::shared_ptr<Prisma::FileBrowser> m_fileBrowser;

		glm::mat4 m_projection;

		std::shared_ptr<Prisma::Effects> m_effects;

    };
}