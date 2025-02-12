#include "../../include/Components/MaterialComponent.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/CacheScene.h"
#include "../../include/Helpers/WindowsHelper.h"

static unsigned int materialId = 0;

void Prisma::MaterialComponent::diffuse(std::vector<Texture> diffuse)
{
	m_diffuse = diffuse;
	CacheScene::getInstance().updateTextures(true);
}

void Prisma::MaterialComponent::normal(std::vector<Texture> normal)
{
	m_normal = normal;
	CacheScene::getInstance().updateTextures(true);
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::diffuse()
{
	return m_diffuse;
}


void Prisma::MaterialComponent::ui()
{
	Prisma::Component::ui();

	auto getLast = [](std::string s)
	{
		size_t found = s.find_last_of('/');
		return found != std::string::npos ? s.substr(found + 1) : s;
	};

	m_diffuseButton = [&]() {
			std::vector<Texture> diffuseTextures;
			Texture texture;
			auto openFolder = Prisma::WindowsHelper::getInstance().openFolder("All Files");
			if (!openFolder.empty()) {
				texture.loadTexture({openFolder,true});
				texture.name(openFolder);
				diffuseTextures.push_back(texture);
				diffuse(diffuseTextures);
			}
		};
	m_normalButton = [&]() {
		std::vector<Texture> normalTextures;
		Texture texture;
		auto openFolder = Prisma::WindowsHelper::getInstance().openFolder("All Files");
		if (!openFolder.empty()) {
			texture.loadTexture({ openFolder });
			texture.name(openFolder);
			normalTextures.push_back(texture);
			normal(normalTextures);
		}
		};
	m_metalnessRoughnessButton = [&]() {
		std::vector<Texture> metalnessRoughnessTextures;
		Texture texture;
		auto openFolder = Prisma::WindowsHelper::getInstance().openFolder("All Files");
		if (!openFolder.empty()) {
			texture.loadTexture({ openFolder });
			texture.name(openFolder);
			metalnessRoughnessTextures.push_back(texture);
			roughnessMetalness(metalnessRoughnessTextures);
		}
		};
	m_specularButton = [&]() {
		std::vector<Texture> specularTextures;
		Texture texture;
		auto openFolder = Prisma::WindowsHelper::getInstance().openFolder("All Files");
		if (!openFolder.empty()) {
			texture.loadTexture({ openFolder });
			texture.name(openFolder);
			specularTextures.push_back(texture);
			specular(specularTextures);
		}
		};
	m_ambientOcclusionButton = [&]() {
		std::vector<Texture> ambientOcclusionTextures;
		Texture texture;
		auto openFolder = Prisma::WindowsHelper::getInstance().openFolder("All Files");
		if (!openFolder.empty()) {
			texture.loadTexture({ openFolder });
			texture.name(openFolder);
			ambientOcclusionTextures.push_back(texture);
			ambientOcclusion(ambientOcclusionTextures);
		}
		};

	if (m_diffuse.size() > 0)
	{
		m_diffuseName = std::make_shared<std::string>(getLast(m_diffuse[0].name()));
		m_componentTypeDiffuse = std::make_tuple(TYPES::BUTTON, "Diffuse", &m_diffuseButton);
		addGlobal(m_componentTypeDiffuse);
	}
	if (m_normal.size() > 0)
	{
		m_normalName = std::make_shared<std::string>(getLast(m_normal[0].name()));
		m_componentTypeNormal = std::make_tuple(TYPES::BUTTON, "Normal", &m_normalButton);
		addGlobal(m_componentTypeNormal);
	}
	if (m_roughnessMetalness.size() > 0)
	{
		m_metalness_roughnessName = std::make_shared<std::string>(getLast(m_roughnessMetalness[0].name()));
		m_componentTypeMetalnessRoughness = std::make_tuple(TYPES::BUTTON, "Metalness-Roughness", &m_metalnessRoughnessButton);
		addGlobal(m_componentTypeMetalnessRoughness);
	}
	if (m_specular.size() > 0)
	{
		m_specularName = std::make_shared<std::string>(getLast(m_specular[0].name()));
		m_componentTypeSpecular = std::make_tuple(TYPES::BUTTON, "Specular", &m_specularButton);
		addGlobal(m_componentTypeSpecular);
	}
	if (m_ambientOcclusion.size() > 0)
	{
		m_ambientOcclusionName = std::make_shared<std::string>(getLast(m_ambientOcclusion[0].name()));
		m_componentTypeAmbientOcclusion = std::make_tuple(TYPES::BUTTON, "AmbientOcclusion", &m_ambientOcclusionButton);
		addGlobal(m_componentTypeAmbientOcclusion);
	}

	ComponentType componentPlain;
	componentPlain = std::make_tuple(TYPES::BOOL, "Plain", &m_plain);

	ComponentType componentTransparent;
	componentTransparent = std::make_tuple(TYPES::BOOL, "Transparent", &m_transparent);

	ComponentType componentColor;
	componentColor = std::make_tuple(TYPES::COLOR, "Color", &m_color);

	m_apply = []()
		{
			CacheScene::getInstance().updateTextures(true);
			CacheScene::getInstance().updateLights(true);
			CacheScene::getInstance().updateStatus(true);
		};

	ComponentType componentButton;
	componentButton = std::make_tuple(TYPES::BUTTON, "Apply", &m_apply);

	addGlobal(componentPlain);
	addGlobal(componentTransparent);
	addGlobal(componentColor);
	addGlobal(componentButton);

	uiRemovable(false);

}

void Prisma::MaterialComponent::update()
{
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::normal()
{
	return m_normal;
}

void Prisma::MaterialComponent::roughnessMetalness(std::vector<Texture> roughnessMetalness)
{
	m_roughnessMetalness = roughnessMetalness;
	CacheScene::getInstance().updateTextures(true);
}

void Prisma::MaterialComponent::specular(std::vector<Texture> specular)
{
	m_specular = specular;
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::specular()
{
	return m_specular;
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::roughnessMetalness()
{
	return m_roughnessMetalness;
}

void Prisma::MaterialComponent::ambientOcclusion(std::vector<Texture> ambientOcclusion)
{
	m_ambientOcclusion = ambientOcclusion;
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::ambientOcclusion()
{
	return m_ambientOcclusion;
}

unsigned int Prisma::MaterialComponent::material_id()
{
	return m_id;
}

void Prisma::MaterialComponent::material_name(std::string name)
{
	m_materialName = name;
}

Prisma::MaterialComponent::MaterialComponent()
{
	name("Material Component");
	m_id = materialId;
	materialId++;
}

std::string Prisma::MaterialComponent::material_name()
{
	return m_materialName;
}

void Prisma::MaterialComponent::transparent(bool transparent)
{
	m_transparent = transparent;
}

bool Prisma::MaterialComponent::transparent() const
{
	return m_transparent;
}

void Prisma::MaterialComponent::color(glm::vec4 color) {
	m_color = color;
	CacheScene::getInstance().updateTextures(true);
}

glm::vec4 Prisma::MaterialComponent::color() const {
	return m_color;
}

void Prisma::MaterialComponent::plain(bool plain)
{
	m_plain = plain;
	CacheScene::getInstance().updateStatus(true);
}

bool Prisma::MaterialComponent::plain()
{
	return m_plain;
}
