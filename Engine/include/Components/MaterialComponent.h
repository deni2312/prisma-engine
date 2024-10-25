#pragma once
#include <vector>
#include "../Containers/Texture.h"
#include "Component.h"

namespace Prisma {

	struct MaterialData {
		uint64_t diffuse;
		uint64_t normal;
		uint64_t roughness_metalness;
        uint64_t specular;
        uint64_t ambient_occlusion;
        bool transparent;
        float padding;
	};

	class MaterialComponent : public Prisma::Component{

    public:
        void ui() override;

        void update() override;

        void diffuse(std::vector<Prisma::Texture> diffuse);

        const std::vector<Prisma::Texture>& diffuse() const;

        void normal(std::vector<Prisma::Texture> normal);

        const std::vector<Prisma::Texture>& normal() const;

        void roughness_metalness(std::vector<Prisma::Texture> roughness_metalness);

        const std::vector<Prisma::Texture>& roughness_metalness() const;

        void specular(std::vector<Prisma::Texture> specular);

        const std::vector<Prisma::Texture>& specular() const;

        void ambientOcclusion(std::vector<Prisma::Texture> ambientOcclusion);

        const std::vector<Prisma::Texture>& ambientOcclusion() const;

        unsigned int material_id();

        void material_name(std::string name);

        MaterialComponent();

        std::string material_name();

        void transparent(bool transparent);

        bool transparent();

    private:

		std::vector<Prisma::Texture> m_diffuse;
		std::vector<Prisma::Texture> m_normal;
		std::vector<Prisma::Texture> m_roughness_metalness;
        std::vector<Prisma::Texture> m_specular;
        std::vector<Prisma::Texture> m_ambientOcclusion;

        std::shared_ptr<std::string> m_diffuseName;
        std::shared_ptr<std::string> m_normalName;
        std::shared_ptr<std::string> m_metalness_roughnessName;
        std::shared_ptr<std::string> m_specularName;
        std::shared_ptr<std::string> m_ambientOcclusionName;

        bool m_transparent = false;

        unsigned int m_id;

        std::string m_materialName;

        ComponentType m_componentTypeDiffuse;
        ComponentType m_componentTypeNormal;
        ComponentType m_componentTypeMetalnessRoughness;
        ComponentType m_componentTypeSpecular;
        ComponentType m_componentTypeAmbientOcclusion;
	};
}