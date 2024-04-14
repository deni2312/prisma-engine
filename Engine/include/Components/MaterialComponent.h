#pragma once
#include <vector>
#include "../Containers/Texture.h"
#include "Component.h"

namespace Prisma {

	struct MaterialData {
		uint64_t diffuse;
		uint64_t normal;
		uint64_t roughness_metalness;
		glm::vec2 padding;
	};

	class MaterialComponent : public Prisma::Component{

    public:
        void start() override;

        void update() override;

        void diffuse(std::vector<Prisma::Texture> diffuse);

        std::vector<Prisma::Texture> diffuse();

        void normal(std::vector<Prisma::Texture> normal);

        std::vector<Prisma::Texture> normal();

        void roughness_metalness(std::vector<Prisma::Texture> roughness_metalness);

        std::vector<Prisma::Texture> roughness_metalness();

    private:

		std::vector<Prisma::Texture> m_diffuse;
		std::vector<Prisma::Texture> m_normal;
		std::vector<Prisma::Texture> m_roughness_metalness;

        std::shared_ptr<std::string> m_diffuseName;
        std::shared_ptr<std::string> m_normalName;
        std::shared_ptr<std::string> m_metalness_roughnessName;

        ComponentType m_componentTypeDiffuse;
        ComponentType m_componentTypeNormal;
        ComponentType m_componentTypeMetalnessRoughness;
	};
}