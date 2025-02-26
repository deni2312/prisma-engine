#include "../include/ImGuiDebug.h"
#include "../../Engine/include/Helpers/ScenePrinter.h"
#include "../../Engine/include/GlobalData/PrismaFunc.h"
#include "../../Engine/include/GlobalData/Keys.h"

#include <memory>
#include <Windows.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm/gtc/type_ptr.hpp"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "../../Engine/include/Helpers/PrismaMath.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"
#include "../../Engine/include/Helpers/PrismaRender.h"
#include "../../GUI/include/ImGuiStyle.h"
#include "../../Engine/include/Postprocess/Postprocess.h"
#include "../include/TextureInfo.h"
#include "../include/PixelCapture.h"
#include "ImGuizmo.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/engine.h"
#include "../../Engine/include/Pipelines/PipelineSkybox.h"
#include "implot.h"
#include "../../Engine/include/Handlers/LoadingHandler.h"
#include "../../Engine/include/Helpers/StringHelper.h"
#include "../../Engine/include/Helpers/WindowsHelper.h"
#include "../include/NodeViewer.h"

struct PrivateIO
{
	ImGuiIO io;
};

std::shared_ptr<PrivateIO> data;

Prisma::ImguiDebug::ImguiDebug() : m_lastFrameTime{glfwGetTime()}, m_fps{60.0f}
{
	data = std::make_shared<PrivateIO>();
	m_camera = std::make_shared<Camera>();
	Engine::getInstance().mainCamera(m_camera);
	m_imguiCamera.mouseCallback();
	m_imguiCamera.mouseButtonCallback();
	Engine::getInstance().setCallback(m_imguiCamera.callback());
	auto settings = SettingsLoader::getInstance().getSettings();
	m_globalSize.x = settings.width / 1920.0f;
	m_globalSize.y = settings.height / 1080.0f;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	data->io = ImGui::GetIO();
	data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	//ImGui::StyleColorsDark();
	ImGuiStyles::darkMode();
	ImGui_ImplGlfw_InitForOpenGL(PrismaFunc::getInstance().window(), true);
	ImGui_ImplOpenGL3_Init("#version 150");
	FBO::FBOData fboData;
	m_height = settings.height;
	m_width = settings.width;
	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.name = "DEBUG";
	m_fbo = std::make_shared<FBO>(fboData);
	Prisma::GlobalData::getInstance().fboTarget(m_fbo);
	m_shader = std::make_shared<Shader>("../../../GUI/Shaders/Output/vertex.glsl",
	                                    "../../../GUI/Shaders/Output/fragment.glsl");
	m_shader->use();
	m_bindlessPos = m_shader->getUniformPosition("screenTexture");
	m_modelPos = m_shader->getUniformPosition("model");
	m_scale = 0.72f;
	m_translate = 1.0f - m_scale;
	m_projection = glm::perspective(
		glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
		static_cast<float>(settings.width) / static_cast<float>(settings.height),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane());
	m_model = translate(glm::mat4(1.0f), glm::vec3(0.0f, m_translate, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(m_scale));

	m_fileBrowser = std::make_shared<FileBrowser>();
	PixelCapture::getInstance();

	m_runButton = std::make_shared<Texture>();
	m_runButton->loadTexture({"../../../GUI/icons/run.png", false, false, false});

	m_pauseButton = std::make_shared<Texture>();
	m_pauseButton->loadTexture({"../../../GUI/icons/pause.png", false, false, false});

	NodeViewer::getInstance();

	initStatus();
}

void Prisma::ImguiDebug::drawGui()
{
	glDisable(GL_DEPTH_TEST);
	float windowWidth = m_translate * m_width / 2;
	ImVec2 size;
	auto nextLeft = [&](float pos)
	{
		ImGui::SetNextWindowPos(ImVec2(0, pos));
		ImGui::SetNextWindowSize(ImVec2(windowWidth, m_height * m_scale - size.y));
	};
	bool openSettings = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				std::string scene = Prisma::WindowsHelper::getInstance().openFolder("All Files");
				if (scene != "")
				{
					Engine::getInstance().getScene(scene, {true});
				}
			}

			if (ImGui::MenuItem("Save"))
			{
				std::string scene = saveFile();
				m_exporter.exportScene(scene);
			}

			if (ImGui::MenuItem("Add model"))
			{
				std::string model = Prisma::WindowsHelper::getInstance().openFolder("All Files");
				if (model != "")
				{
					if (Prisma::StringHelper::getInstance().endsWith(model, ".prisma"))
					{
						if (Prisma::GlobalData::getInstance().currentGlobalScene()->root)
						{
							Prisma::LoadingHandler::getInstance().load(model, {true, nullptr, true});
						}
						else
						{
							Engine::getInstance().getScene(model, {true});
						}
					}
					else
					{
						Prisma::SceneLoader loader;
						auto scene = loader.loadScene(model, {true});
						if (Prisma::GlobalData::getInstance().currentGlobalScene()->root)
						{
							Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(scene->root);
						}
						else
						{
							Prisma::GlobalData::getInstance().currentGlobalScene(scene);
						}
					}

					MeshIndirect::getInstance().init();

					CacheScene::getInstance().updateSizes(true);
				}
			}

			if (ImGui::MenuItem("Add skybox"))
			{
				std::string scene = Prisma::WindowsHelper::getInstance().openFolder("All Files");
				if (scene != "")
				{
					Texture texture;
					texture.loadEquirectangular(scene);
					texture.data({4096, 4096, 3});
					PipelineSkybox::getInstance().texture(texture, true);
				}
			}

			if (ImGui::MenuItem("Settings"))
			{
				openSettings = true;
			}

			if (ImGui::MenuItem("Close"))
			{
				PrismaFunc::getInstance().closeWindow();
			}

			ImGui::EndMenu();
		}
		size = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
	m_initOffset = size.y;
	bool isOpen = true;
	if (!m_run)
	{
		ImGui::SetNextWindowPos(ImVec2(windowWidth, m_initOffset));
		ImGui::SetNextWindowSize(ImVec2(m_width * m_scale, 0));
	}
	else
	{
		ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
		ImGui::SetNextWindowSize(ImVec2(m_width, 0));
	}
	ImGui::Begin("Dummy Top", &isOpen,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse);

	auto positionRun = m_run ? m_width / 2 : m_width * m_scale / 2;

	ImGui::SetCursorPosX(positionRun);

	auto currentButton = m_run ? m_pauseButton : m_runButton;

	if (ImGui::ImageButton((void*)currentButton->id(), ImVec2(24, 24)))
	{
		m_run = !m_run;
		Engine::getInstance().debug(!m_run);
		if (m_run)
		{
			Engine::getInstance().setCallback(Engine::getInstance().getUserEngine()->callbacks());
		}
		else
		{
			Prisma::CacheScene::getInstance().updateSizes(true);
			Engine::getInstance().setCallback(m_imguiCamera.callback());
		}
	}
	m_buttonSize = ImGui::GetWindowSize().y;

	ImGui::End();
	m_model = translate(glm::mat4(1.0f), glm::vec3(0.0f, m_translate - 2*(m_buttonSize+m_initOffset) / static_cast<float>(m_height), 0.0f))* scale(glm::mat4(1.0f), glm::vec3(m_scale));


	if (!m_run)
	{
		ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
		ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

		ImGui::Begin("Dummy Left", &isOpen,
		             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs);
		ImGui::Dummy(ImVec2(0.0f, m_height * m_scale+ m_buttonSize));
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(m_width * m_scale + windowWidth, m_initOffset));
		ImGui::SetNextWindowSize(ImVec2(windowWidth, 0));

		ImGui::Begin("Dummy Right", &isOpen,
		             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
		             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs);
		ImGui::Dummy(ImVec2(0, m_height * m_scale+ m_buttonSize));
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(0, m_initOffset));
		ImGui::SetNextWindowSize(ImVec2(windowWidth, m_height* m_scale + m_buttonSize));
		ImGui::Begin("Scene", nullptr,
		             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar);
		m_plot.showFPS(m_fps);

		if (openSettings)
		{
			ImGui::OpenPopup("SettingsTab");
			openSettings = false;
		}
		m_settingsTab.drawSettings();
		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Text(("UI Time: " + std::to_string(m_timeCounterUI.duration_seconds())).c_str());
		ImGui::Text(("Engine Time: " + std::to_string(m_timeCounterEngine.duration_seconds())).c_str());
		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		ImGui::Separator();

		ImGuiTabs::getInstance().showNodes(Prisma::GlobalData::getInstance().currentGlobalScene()->root,
m_imguiCamera);
		// Check if the node is clicked
		ImGui::End();
		m_fileBrowser->show(m_width, m_height, m_initOffset+m_buttonSize, m_scale, m_translate);
		if (m_imguiCamera.currentSelect())
		{
			auto currentSelectMesh = dynamic_cast<Mesh*>(m_imguiCamera.currentSelect());
			auto currentSelectLightDir = dynamic_cast<Light<LightType::LightDir>*>(m_imguiCamera.currentSelect());
			auto currentSelectLightOmni = dynamic_cast<Light<LightType::LightOmni>*>(m_imguiCamera.currentSelect());
			auto currentSelectLightArea = dynamic_cast<Light<LightType::LightArea>*>(m_imguiCamera.currentSelect());
			NodeViewer::NodeData nodeData;
			nodeData.camera = m_camera;
			nodeData.projection = m_model * m_projection;
			nodeData.translate = m_translate;
			nodeData.width = m_width;
			nodeData.height = m_height;
			nodeData.scale = m_scale;
			nodeData.initOffset = m_initOffset;
			nodeData.node = m_imguiCamera.currentSelect();

			if (currentSelectMesh)
			{
				meshInfo.showSelected(nodeData);
			}
			else if (currentSelectLightDir)
			{
				lightInfo.showSelectedDir(currentSelectLightDir, nodeData);
			}
			else if (currentSelectLightOmni)
			{
				lightInfo.showSelectedOmni(currentSelectLightOmni, nodeData);
			}
			else if (currentSelectLightArea)
			{
				lightInfo.showSelectedArea(currentSelectLightArea, nodeData);
			}
			else
			{
				NodeViewer::getInstance().showSelected(nodeData);
			}
		}

		m_settingsTab.updateStatus();
	}
	drawScene();
	glEnable(GL_DEPTH_TEST);
}

float Prisma::ImguiDebug::fps()
{
	return m_fps;
}

void Prisma::ImguiDebug::start()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void Prisma::ImguiDebug::close()
{
	m_imguiCamera.constraints({
		m_translate * m_width / 2, m_initOffset + 50, m_translate * m_width / 2 + m_scale * m_width, m_height * m_scale,
		ImGuizmo::IsOver(), m_scale, m_model
	});


	if (!m_run)
	{
		double currentFrameTime = glfwGetTime();
		if (currentFrameTime != m_lastFrameTime)
		{
			// Avoid division by zero in the first frame
			m_fps = 1.0f / static_cast<float>(currentFrameTime - m_lastFrameTime);
		}
		m_lastFrameTime = currentFrameTime;

		m_imguiCamera.updateCamera(m_camera);
		m_imguiCamera.keyboardUpdate(PrismaFunc::getInstance().window());
		m_addingMenu.addMenu(m_imguiCamera);
		ImGuiTabs::getInstance().updateTabs(Prisma::GlobalData::getInstance().currentGlobalScene()->root, 0);
	}
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Prisma::ImguiDebug::imguiData(std::shared_ptr<ImGuiData> data)
{
	m_data = data;
}

std::shared_ptr<Prisma::SceneHandler> Prisma::ImguiDebug::handlers()
{
	m_handlers = std::make_shared<SceneHandler>();
	m_handlers->onBeginRender = [&]()
	{
		m_timeCounterEngine.start();
		getInstance().start();
	};
	m_handlers->onLoading = [&](auto data)
	{
		getInstance().onLoading(data);
	};
	m_handlers->onEndRender = [&]()
	{
		m_timeCounterEngine.stop();
		m_timeCounterUI.start();
		getInstance().drawGui();

		getInstance().close();
		m_timeCounterUI.stop();
	};
	return m_handlers;
}

Prisma::ImguiDebug::GlobalSize Prisma::ImguiDebug::globalSize()
{
	return m_globalSize;
}

std::shared_ptr<Prisma::FBO> Prisma::ImguiDebug::fbo()
{
	return m_fbo;
}

void Prisma::ImguiDebug::drawScene()
{
	m_shader->use();
	m_shader->setInt64(m_bindlessPos, m_fbo->texture());

	auto model = glm::mat4(1.0f);
	if (!m_run)
	{
		model = m_model;
	}
	m_shader->setMat4(m_modelPos, model);
	PrismaRender::getInstance().renderQuad();
}

void Prisma::ImguiDebug::initStatus()
{
	m_settingsTab.init();
}

std::string Prisma::ImguiDebug::saveFile()
{
	OPENFILENAME ofn;
	char szFile[260];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All Files\0*.prisma\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	// Open the Save File dialog
	if (GetSaveFileName(&ofn) == TRUE)
	{
		return szFile;
	}
	return "";
}

void Prisma::ImguiDebug::onLoading(std::pair<std::string, int>& data)
{
	// Begin the popup
	ImGui::OpenPopup("Loading...");

	if (ImGui::BeginPopupModal("Loading...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		float progress = 0;
		// Display the name of the task being loaded
		if (data.first.empty())
		{
			ImGui::Text("Reading prisma");
		}
		else
		{
			ImGui::Text("Loading: %s", data.first.c_str());
			progress = static_cast<float>(data.second) / 100.0f; // Convert percentage to fraction
		}
		ImGui::ProgressBar(progress, ImVec2(300, 0)); // ProgressBar(width, height)
		ImGui::EndPopup();
	}
}
