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

		const aiScene* assimpScene();
		const aiScene* m_aScene;

	private:

		float calculateOmniLightRadius(float Kc, float Kl, float Kq, float I_threshold);

		void nodeIteration(std::shared_ptr<Node> nodeRoot,aiNode* node, const aiScene* scene);

		void setVertexBoneDataToDefault(Prisma::AnimatedMesh::AnimateVertex& vertex);

		void setVertexBoneData(Prisma::AnimatedMesh::AnimateVertex& vertex, int boneID, float weight);

		void extractBoneWeightForVertices(std::shared_ptr<Prisma::AnimatedMesh> animatedMesh,std::shared_ptr<AnimatedMesh::AnimateVerticesData> vertices, aiMesh* mesh, const aiScene* scene);

		std::shared_ptr<Prisma::Mesh> getMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Prisma::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,bool srgb=false);
		void loadLights(const aiScene* currentScene, std::shared_ptr<Node> root);

		std::vector<Prisma::Texture> textures_loaded;
		std::shared_ptr<Scene> m_scene;
		std::string m_folder;
		NodeHelper m_nodeFinder;


		SceneParameters m_sceneParameters;
	};
}