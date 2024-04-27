#pragma once
#include "../SceneObjects/Node.h"
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"
#include <memory>
#include <vector>
#include <string>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "../Helpers/NodeHelper.h"
#include "SceneData.h"

namespace Prisma {

	class SceneLoader {
	public:
		struct SceneParameters {
			bool srgb = false;
		};
		std::shared_ptr<Scene> loadScene(std::string scene, SceneParameters sceneParameters);



	private:

		float calculateOmniLightRadius(float Kc, float Kl, float Kq, float I_threshold);

		void nodeIteration(std::shared_ptr<Node> nodeRoot,aiNode* node, const aiScene* scene);
		glm::mat4 getTransform(aiMatrix4x4 matrix);
		std::shared_ptr<Prisma::Mesh> getMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Prisma::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,bool srgb=false);
		void loadLights(const aiScene* currentScene, std::shared_ptr<Node> root);
		std::vector<Prisma::Texture> textures_loaded;
		std::shared_ptr<Scene> m_scene;
		std::string m_folder;
		Prisma::Texture black;
		Prisma::Texture normal;
		NodeHelper nodeFinder;
		SceneParameters m_sceneParameters;
	};
}