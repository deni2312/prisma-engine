#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <functional>
#include <memory>
#include "SceneObjects/Node.h"
#include "SceneObjects/Camera.h"
#include "Helpers/Settings.h"
#include "SceneData/SceneLoader.h"
#include "GlobalData/PrismaFunc.h"
#include "GlobalData/EngineSettings.h"
#include "../../GUI/include/ImGuiDebug.h"

namespace Prisma {
	class Engine {
	public:

		bool run();
		friend class ImguiDebug;
		static Engine& getInstance();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		Engine();

		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		float fps();
		void mainCamera(std::shared_ptr<Camera> camera);
		void engineSettings(const Prisma::EngineSettings::Settings& pipeline);
		Prisma::EngineSettings::Settings engineSettings() const;
		std::shared_ptr<Prisma::Scene> getScene(std::string scene, Prisma::SceneLoader::SceneParameters sceneParameters);
		void setUserEngine(std::shared_ptr<Prisma::UserData> userData);
		std::shared_ptr<Prisma::UserData> getUserEngine();
		void setGuiData(std::shared_ptr<Prisma::SceneHandler> guiData);
		void debug(bool debug);

	private:
		static std::shared_ptr<Engine> instance;
		void initScene();
	};
}