#pragma once
#include "PrismaFunc.h"

#include "../SceneData/SceneData.h"
#include <memory>
#include <unordered_map>

namespace Prisma::LightType
{
	struct LightDir;
	struct LightOmni;
}

namespace Prisma
{
	class GlobalData : public InstanceData<GlobalData>
	{
	public:
		struct GlobalTextureInfo
		{
			Prisma::Texture texture;
			std::string name="";
			glm::vec2 size=glm::vec2(100,100);
		};

		// Getter and Setter for m_currentGlobalScene
		std::shared_ptr<Scene> currentGlobalScene() const;

		void currentGlobalScene(const std::shared_ptr<Scene>& currentGlobalScene);

		// Getter and Setter for m_fboTarget
		std::shared_ptr<FBO> fboTarget() const;

		void fboTarget(const std::shared_ptr<FBO>& fboTarget);

		// Getter and Setter for m_sceneComponents
		std::unordered_map<uint64_t, Component*>& sceneComponents();

		void sceneComponents(const std::unordered_map<uint64_t, Component*>& sceneComponents);

		// Getter and Setter for m_sceneNodes
		std::unordered_map<uint64_t, std::shared_ptr<Node>>& sceneNodes();

		void sceneNodes(const std::unordered_map<uint64_t, std::shared_ptr<Node>>& sceneNodes);

		// Getter and Setter for m_currentProjection
		glm::mat4 currentProjection();

		void currentProjection(const glm::mat4& currentProjection);

		// Getter and Setter for m_defaultBlack
		Texture& defaultBlack();

		void defaultBlack(const Texture& defaultBlack);

		// Getter and Setter for m_defaultWhite
		Texture& defaultWhite();

		void defaultWhite(const Texture& defaultWhite);

		// Getter and Setter for m_defaultNormal
		Texture& defaultNormal();

		void defaultNormal(const Texture& defaultNormal);

		void transparencies(bool transparencies);

		bool transparencies() const;

		void addGlobalTexture(GlobalTextureInfo texture);

		const std::vector<GlobalTextureInfo>& globalTextures();

	private:
		std::shared_ptr<Scene> m_currentGlobalScene;
		std::shared_ptr<FBO> m_fboTarget;
		std::unordered_map<uint64_t, Component*> m_sceneComponents;
		std::unordered_map<uint64_t, std::shared_ptr<Node>> m_sceneNodes;
		std::vector<GlobalTextureInfo> m_textures;
		glm::mat4 m_currentProjection;
		Texture m_defaultBlack;
		Texture m_defaultWhite;
		Texture m_defaultNormal;
		bool m_transparencies = true;
	};
}
