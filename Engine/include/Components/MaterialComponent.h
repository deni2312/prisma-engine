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
		glm::vec4 color = glm::vec4(0);
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

		void roughnessMetalness(std::vector<Texture> roughnessMetalness);

		std::vector<Texture>& roughnessMetalness();

		void specular(std::vector<Texture> specular);

		std::vector<Texture>& specular();

		void ambientOcclusion(std::vector<Texture> ambientOcclusion);

		std::vector<Texture>& ambientOcclusion();

		unsigned int material_id();

		void material_name(std::string name);

		MaterialComponent();

		std::string material_name();

		void transparent(bool transparent);

		bool transparent() const;

		void color(glm::vec4 color);

		glm::vec4 color() const;

		void plain(bool plain);

		bool plain();

	private:
		std::vector<Texture> m_diffuse;
		std::vector<Texture> m_normal;
		std::vector<Texture> m_roughnessMetalness;
		std::vector<Texture> m_specular;
		std::vector<Texture> m_ambientOcclusion;

		std::shared_ptr<std::string> m_diffuseName;
		std::shared_ptr<std::string> m_normalName;
		std::shared_ptr<std::string> m_metalness_roughnessName;
		std::shared_ptr<std::string> m_specularName;
		std::shared_ptr<std::string> m_ambientOcclusionName;

		glm::vec4 m_color=glm::vec4(0);

		bool m_plain = false;

		bool m_transparent = false;

		unsigned int m_id;

		std::string m_materialName;

		ComponentType m_componentTypeDiffuse;
		ComponentType m_componentTypeNormal;
		ComponentType m_componentTypeMetalnessRoughness;
		ComponentType m_componentTypeSpecular;
		ComponentType m_componentTypeAmbientOcclusion;
		std::function<void()> m_apply;
		std::function<void()> m_diffuseButton;
		std::function<void()> m_normalButton;
		std::function<void()> m_metalnessRoughnessButton;
		std::function<void()> m_specularButton;
		std::function<void()> m_ambientOcclusionButton;

		glm::vec2 m_size=glm::vec2(90,90);
	};
}
