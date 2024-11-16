#pragma once
#include <vector>
#include "../Containers/Texture.h"
#include "Component.h"

namespace Prisma
{
	struct MaterialData
	{
		uint64_t diffuse;
		uint64_t normal;
		uint64_t roughness_metalness;
		uint64_t specular;
		uint64_t ambient_occlusion;
		int transparent;
		float padding;
	};

	class MaterialComponent : public Component
	{
	public:
		void ui() override;

		void update() override;

		void diffuse(std::vector<Texture> diffuse);

		std::vector<Texture>& diffuse();

		void normal(std::vector<Texture> normal);

		std::vector<Texture>& normal();

		void roughness_metalness(std::vector<Texture> roughness_metalness);

		std::vector<Texture>& roughness_metalness();

		void specular(std::vector<Texture> specular);

		std::vector<Texture>& specular();

		void ambientOcclusion(std::vector<Texture> ambientOcclusion);

		std::vector<Texture>& ambientOcclusion();

		unsigned int material_id();

		void material_name(std::string name);

		MaterialComponent();

		std::string material_name();

		void transparent(bool transparent);

		bool transparent();

	private:
		std::vector<Texture> m_diffuse;
		std::vector<Texture> m_normal;
		std::vector<Texture> m_roughness_metalness;
		std::vector<Texture> m_specular;
		std::vector<Texture> m_ambientOcclusion;

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
