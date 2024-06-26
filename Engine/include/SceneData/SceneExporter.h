#include <memory>
#include "../SceneObjects/Node.h"
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include "../SceneObjects/Mesh.h"
#include "../SceneObjects/Light.h"

namespace Prisma {
	class Exporter {
	public:

		Exporter();

		void exportScene();

		static Exporter& getInstance();
		Exporter(const Exporter&) = delete;
		Exporter& operator=(const Exporter&) = delete;
	private:
		static std::shared_ptr<Exporter> instance;
        void addNodesRecursively(const std::shared_ptr<Prisma::Node>& sceneNode,aiNode* next);
		const aiScene* m_scene;

		aiMesh* getMesh(std::shared_ptr<Prisma::Mesh> mesh);
		
		aiLight* getLightDir(std::shared_ptr<Prisma::Light<Prisma::LightType::LightDir>> light);
		aiLight* getLightOmni(std::shared_ptr<Prisma::Light<Prisma::LightType::LightOmni>> light);

		aiMatrix4x4 glmToAiMatrix4x4(const glm::mat4& from);

		unsigned int materialIndex(std::shared_ptr<MaterialComponent> material);
	};
}