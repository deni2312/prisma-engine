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
#include "../../GUI/include/ImGuiDebug.h"

namespace Prisma {
	class Engine {
	public:
		enum class Pipeline {
			FORWARD,
			DEFERRED
		};
		Engine(SceneHandler sceneHandler = {});
		bool run();
		friend class ImguiDebug;
	private:
		void initScene();
		void loadNewScene();
	protected:
		void setCallback(std::shared_ptr<CallbackHandler> callbackHandler);
		virtual bool update();
		float fps();
		Settings settings();
		void mainCamera(std::shared_ptr<Camera> camera);
		void pipeline(Pipeline pipeline);
		Pipeline pipeline();
		std::shared_ptr<Prisma::Scene> getScene(std::string scene,Prisma::SceneLoader::SceneParameters sceneParameters);
	};
}