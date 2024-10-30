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

namespace Prisma
{
	class Engine : public InstanceData<Engine>
	{
	public:
		bool run();

		Engine();

		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		float fps();
		void mainCamera(const std::shared_ptr<Camera>& camera);
		void engineSettings(const EngineSettings::Settings& pipeline);
		EngineSettings::Settings engineSettings() const;
		std::shared_ptr<Scene> getScene(const std::string& scene,
		                                SceneLoader::SceneParameters sceneParameters);
		void setUserEngine(std::shared_ptr<UserData> userData);
		std::shared_ptr<UserData> getUserEngine();
		void setGuiData(std::shared_ptr<SceneHandler> guiData);
		void debug(bool debug);

	private:
		void initScene();
	};
}
