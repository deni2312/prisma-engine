#include "GlobalData/GlobalData.h"
#include "Helpers/SettingsLoader.h"
#include "ImGuiDebug.h"
#include "ImGuiStyle.h"
#include "ImGuiKey.h"
#include "ThirdParty/imgui/imgui.h"
#include "SceneObjects/Camera.h"
#include <Windows.h>

#include "engine.h"
#include "Handlers/LoadingHandler.h"
#include "Helpers/StringHelper.h"
#include "Pipelines/PipelineSkybox.h"
#include "SceneData/MeshIndirect.h"
#include "ImGuiTabs.h"
#include "NodeViewer.h"
#include "ScenePipeline.h"
#include "Imgui/interface/ImGuiImplWin32.hpp"
#include "../imguizmo/imguizmo.h"
#include "PixelCapture.h"


struct PrivateIO
{
	ImGuiIO io;
};

std::shared_ptr<PrivateIO> data;

Prisma::ImguiDebug::ImguiDebug() : m_lastFrameTime{glfwGetTime()}, m_fps{60.0f}
{

	data = std::make_shared<PrivateIO>();
	m_camera = std::make_shared<Prisma::Camera>();
	Prisma::Engine::getInstance().mainCamera(m_camera);
	Prisma::PrismaFunc::UIInput uiInput;
	uiInput.mouseClick = [&](int button,int action)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(button, action);
	};
	uiInput.keyboard = [&](int key, int action,int scancode)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(Prisma::GUI::convertImGuiGlfwKey(key,scancode), (action== GLFW_PRESS));
		if (action == GLFW_PRESS && key >= 32 && key <= 126) {
			io.AddInputCharacter(key);
		}
	};

	uiInput.mouseRoll = [&](int x, int y)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(x, y);
	};

	Prisma::PrismaFunc::getInstance().inputUI(uiInput);
	m_imguiCamera.mouseCallback();
	m_imguiCamera.mouseButtonCallback();
	Engine::getInstance().setCallback(m_imguiCamera.callback());
	auto settings = Prisma::SettingsLoader::getInstance().getSettings();
	m_globalSize.x = settings.width / 1920.0f;
	m_globalSize.y = settings.height / 1080.0f;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	//ImPlot::CreateContext();
	data->io = ImGui::GetIO();
	data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	data->io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	//ImGui::StyleColorsDark();
	Prisma::ImGuiStyles::getInstance().darkMode();
	m_height = settings.height;
	m_width = settings.width;
	m_scale = 0.72f;
	m_translate = 1.0f - m_scale;
	m_projection = glm::perspective(
		glm::radians(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->angle()),
		static_cast<float>(settings.width) / static_cast<float>(settings.height),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane(),
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane());
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	m_model = translate(glm::mat4(1.0f), glm::vec3(0.0f, m_translate, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(m_scale));

	m_fileBrowser = std::make_shared<FileBrowser>();

	m_runButton = std::make_shared<Texture>();
	m_runButton->loadTexture({"../../../GUI/icons/run.png",false});

	m_pauseButton = std::make_shared<Texture>();
	m_pauseButton->loadTexture({"../../../GUI/icons/pause.png",false});

	NodeViewer::getInstance();
	m_imguiDiligent = Diligent::ImGuiImplWin32::Create(Diligent::ImGuiDiligentCreateInfo{ contextData.m_pDevice ,contextData.m_pSwapChain->GetDesc() }, (HWND)Prisma::PrismaFunc::getInstance().windowNative());

	Prisma::ScenePipeline::getInstance();
	Prisma::PixelCapture::getInstance();
	initStatus();
}

void Prisma::ImguiDebug::drawGui()
{
	m_translate = 1.0f - m_scale;
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
				auto children = Prisma::GlobalData::getInstance().currentGlobalScene()->root->children();
				for (const auto& child : children)
				{
					Prisma::GlobalData::getInstance().currentGlobalScene()->root->removeChild(child->uuid());
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
							Prisma::LoadingHandler::getInstance().load(model, { true, nullptr, true });
						}
						else
						{
							Prisma::LoadingHandler::getInstance().load(model, { true, nullptr, false });
						}
					}
					else
					{
						Prisma::SceneLoader loader;
						auto scene = loader.loadScene(model, { true });
						if (Prisma::GlobalData::getInstance().currentGlobalScene()->root)
						{
							Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(scene->root);
						}
						else
						{
							Prisma::GlobalData::getInstance().currentGlobalScene(scene);
						}
					}
					Prisma::MeshIndirect::getInstance().init();
					CacheScene::getInstance().updateSizes(true);
				}
			}

			if (ImGui::MenuItem("Add skybox"))
			{
				std::string scene = Prisma::WindowsHelper::getInstance().openFolder("All Files");
				if (scene != "")
				{
					Texture texture;
					texture.loadTexture({ scene,true });
					Prisma::PipelineSkybox::getInstance().texture(texture);
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

	auto& query = Prisma::QueryGPU::getInstance().queryData();

	if (ImGui::Button("Show Stats",ImVec2(0,36))) {
		// Button logic if needed
	}
	ImGui::SameLine();
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip(); // Start tooltip when hovering

		if (ImGui::BeginTable("StatsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
			ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			// Add each row manually
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("UI Time:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%.9f", m_timeCounterUI.duration_seconds());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Engine Time:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%.9f", m_timeCounterEngine.duration_seconds());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Meshes:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Animate Meshes:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Omni Lights:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", Prisma::GlobalData::getInstance().currentGlobalScene()->omniLights.size());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Dir Lights:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", Prisma::GlobalData::getInstance().currentGlobalScene()->dirLights.size());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Area Lights:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%zu", Prisma::GlobalData::getInstance().currentGlobalScene()->areaLights.size());
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Input Vertices:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%d", query.PipelineStats.InputVertices);
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Input Primitives:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%d", query.PipelineStats.InputPrimitives);
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("VS Invocations:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%d", query.PipelineStats.VSInvocations);
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Clipping Invocations:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%d", query.PipelineStats.ClippingInvocations);
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Clipping Primitives:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%d", query.PipelineStats.ClippingPrimitives);
			ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("PS Invocations:"); ImGui::TableSetColumnIndex(1); ImGui::Text("%d", query.PipelineStats.PSInvocations);

			ImGui::EndTable();
		}

		ImGui::EndTooltip(); // End tooltip
	}


	ImGui::SetCursorPosX(positionRun);

	auto currentButton = m_run ? m_pauseButton : m_runButton;
	if (ImGui::ImageButton((void*)currentButton->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_TYPE::TEXTURE_VIEW_SHADER_RESOURCE), ImVec2(24, 24)))
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
	m_model = translate(glm::mat4(1.0f), glm::vec3(0.0f, m_translate - 2*(m_buttonSize+m_initOffset) / static_cast<float>(m_height), 0.0f))* glm::scale(glm::mat4(1.0f), glm::vec3(m_scale));


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
		//m_plot.showFPS(m_fps);

		if (openSettings)
		{
			ImGui::OpenPopup("SettingsTab");
			openSettings = false;
		}
		m_settingsTab.drawSettings();
		Prisma::ImGuiTabs::getInstance().showNodes(Prisma::GlobalData::getInstance().currentGlobalScene()->root,m_imguiCamera);
		// Check if the node is clicked
		ImGui::End();
		m_fileBrowser->show(m_width, m_height, m_initOffset+m_buttonSize, m_scale, m_translate);
		if (m_imguiCamera.currentSelect())
		{
			auto currentSelectMesh = dynamic_cast<Mesh*>(m_imguiCamera.currentSelect());
			auto currentSelectLightDir = dynamic_cast<Light<LightType::LightDir>*>(m_imguiCamera.currentSelect());
			auto currentSelectLightOmni = dynamic_cast<Light<LightType::LightOmni>*>(m_imguiCamera.currentSelect());
			auto currentSelectLightArea = dynamic_cast<Light<LightType::LightArea>*>(m_imguiCamera.currentSelect());
			Prisma::NodeViewer::NodeData nodeData;
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
	//glEnable(GL_DEPTH_TEST);
}

float Prisma::ImguiDebug::fps()
{
	return m_fps;
}

void Prisma::ImguiDebug::start()
{
	//ImGui_ImplOpenGL3_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//
	auto contextDesc = Prisma::PrismaFunc::getInstance().contextData().m_pSwapChain->GetDesc();
	m_imguiDiligent->NewFrame(m_width,m_height, contextDesc.PreTransform);
	ImGuizmo::BeginFrame();
}

void Prisma::ImguiDebug::close()
{

	m_imguiCamera.constraints({
		m_translate * m_width / 2, m_initOffset + 50, m_translate * m_width / 2 + m_scale * m_width, m_height * m_scale,
		false, m_scale, m_model
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
	m_imguiDiligent->Render(Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext);
	//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
		Prisma::QueryGPU::getInstance().start();
	};
	m_handlers->onLoading = [&](auto data)
	{
		getInstance().onLoading(data);
	};
	m_handlers->onEndRender = [&]()
	{
		Prisma::QueryGPU::getInstance().end();
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

void Prisma::ImguiDebug::scale(float scale)
{
	m_scale = scale;
}

float Prisma::ImguiDebug::scale()
{
	return m_scale;
}
//
//std::shared_ptr<Prisma::FBO> Prisma::ImguiDebug::fbo()
//{
//	return m_fbo;
//}

void Prisma::ImguiDebug::drawScene()
{
	auto model = glm::mat4(1.0f);
	if (!m_run)
	{
		model = m_model;
	}
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

	auto pRTV = contextData.m_pSwapChain->GetCurrentBackBufferRTV();
	auto pDSV = contextData.m_pSwapChain->GetDepthBufferDSV();
	Prisma::ScenePipeline::getInstance().render(model,pRTV,pDSV);
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
