#include "../../include/Components/MaterialComponent.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/CacheScene.h"

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
	if (m_diffuse.size() > 0)
	{
		m_diffuseName = std::make_shared<std::string>(getLast(m_diffuse[0].name()));
		m_componentTypeDiffuse = std::make_tuple(TYPES::STRING, "Diffuse", m_diffuseName.get());
		addGlobal(m_componentTypeDiffuse);
	}
	if (m_normal.size() > 0)
	{
		m_normalName = std::make_shared<std::string>(getLast(m_normal[0].name()));
		m_componentTypeNormal = std::make_tuple(TYPES::STRING, "Normal", m_normalName.get());
		addGlobal(m_componentTypeNormal);
	}
	if (m_roughness_metalness.size() > 0)
	{
		m_metalness_roughnessName = std::make_shared<std::string>(getLast(m_roughness_metalness[0].name()));
		m_componentTypeMetalnessRoughness = std::make_tuple(TYPES::STRING, "Metalness-Roughness",
		                                                    m_metalness_roughnessName.get());
		addGlobal(m_componentTypeMetalnessRoughness);
	}
	if (m_specular.size() > 0)
	{
		m_specularName = std::make_shared<std::string>(getLast(m_specular[0].name()));
		m_componentTypeSpecular = std::make_tuple(TYPES::STRING, "Specular", m_specularName.get());
		addGlobal(m_componentTypeSpecular);
	}
	if (m_ambientOcclusion.size() > 0)
	{
		m_ambientOcclusionName = std::make_shared<std::string>(getLast(m_ambientOcclusion[0].name()));
		m_componentTypeAmbientOcclusion = std::make_tuple(TYPES::STRING, "Diffuse", m_ambientOcclusionName.get());
		addGlobal(m_componentTypeAmbientOcclusion);
	}

	uiRemovable(false);

	m_id = materialId;
	materialId++;
}

void Prisma::MaterialComponent::update()
{
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::normal()
{
	return m_normal;
}

void Prisma::MaterialComponent::roughness_metalness(std::vector<Texture> roughness_metalness)
{
	m_roughness_metalness = roughness_metalness;
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

std::vector<Prisma::Texture>& Prisma::MaterialComponent::roughness_metalness()
{
	return m_roughness_metalness;
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
