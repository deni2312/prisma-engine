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
#include "SceneExporter.h"

namespace Prisma
{
	class SceneLoader
	{
	public:
		struct SceneParameters
		{
			bool srgb = false;
			std::function<void(std::shared_ptr<Scene>)> onLoad = nullptr;
			bool append = false;
		};

		std::shared_ptr<Scene> loadScene(std::string scene, SceneParameters sceneParameters);

		void loadSceneAsync(std::string scene, SceneParameters sceneParameters);

		std::shared_ptr<Scene> hasFinish();

		const aiScene* assimpScene();
		const aiScene* m_aScene;

		Prisma::Exporter& exporter();

		void onLoading(std::function<void()> loading);

	private:
		float calculateOmniLightRadius(float Kc, float Kl, float Kq, float I_threshold);

		void nodeIteration(std::shared_ptr<Node> nodeRoot, aiNode* node, const aiScene* scene);

		void setVertexBoneDataToDefault(AnimatedMesh::AnimateVertex& vertex);

		void setVertexBoneData(AnimatedMesh::AnimateVertex& vertex, int boneID, float weight);

		void extractBoneWeightForVertices(std::shared_ptr<AnimatedMesh> animatedMesh,
		                                  std::shared_ptr<AnimatedMesh::AnimateVerticesData> vertices, aiMesh* mesh,
		                                  const aiScene* scene);
		Prisma::Exporter m_exporter;

		std::shared_ptr<Mesh> getMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, bool srgb = false);
		void loadLights(const aiScene* currentScene, std::shared_ptr<Node> root);

		std::map<std::string, Texture> m_texturesLoaded;
		std::shared_ptr<Scene> m_scene;
		std::string m_folder;
		NodeHelper m_nodeFinder;

		std::function<void()> m_loading;

		SceneParameters m_sceneParameters;
	};
}
