#pragma once
#include "../SceneObjects/Node.h"
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/AnimatedMesh.h"
#include "../SceneObjects/Light.h"
#include "../SceneObjects/Camera.h"
#include <vector>
#include <string>
#include "../Containers/FBO.h"

namespace Prisma {
	template <typename T>
	class Light;
	namespace LightType {
		struct LightDir;
		struct LightOmni;
	}

	struct SceneHandler {
		std::function<void()> onBeginRender = []() {};
		std::function<void()> onEndRender = []() {};
	};

	struct UserData {
		virtual void start() = 0;
		virtual void update() = 0;
		virtual void finish() = 0;
		virtual std::shared_ptr<CallbackHandler> callbacks() = 0;
	};

	struct Scene {

		struct ModelData {
			std::vector<unsigned int> offsets;
			std::vector<glm::mat4> models;
		};

		std::shared_ptr<Prisma::Node> root;
		std::shared_ptr<Prisma::Camera> camera;
		std::vector<std::shared_ptr<Prisma::Mesh>> meshes;
		std::vector<std::shared_ptr<Prisma::AnimatedMesh>> animateMeshes;
		ModelData modelData;
		std::vector<std::shared_ptr<Prisma::Light<Prisma::LightType::LightDir>>> dirLights;
		std::vector<std::shared_ptr<Prisma::Light<Prisma::LightType::LightOmni>>> omniLights;
		std::string name;
	};
}