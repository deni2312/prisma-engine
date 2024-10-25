#include "../../include/Components/MaterialComponent.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/CacheScene.h"

static unsigned int materialId = 0;

void Prisma::MaterialComponent::diffuse(std::vector <Prisma::Texture> diffuse) {
    m_diffuse=diffuse;
    Prisma::CacheScene::getInstance().updateTextures(true);
}

void Prisma::MaterialComponent::normal(std::vector <Prisma::Texture> normal) {
    m_normal=normal;
    Prisma::CacheScene::getInstance().updateTextures(true);
}

const std::vector <Prisma::Texture>& Prisma::MaterialComponent::diffuse() const {
    return m_diffuse;
}

void Prisma::MaterialComponent::ui() {
    auto getLast=[](std::string s){
        size_t found = s.find_last_of('/');
        return found != std::string::npos ? s.substr(found + 1) : s;
    };
    m_diffuseName = std::make_shared<std::string>(getLast(m_diffuse[0].name()));
    m_normalName = std::make_shared<std::string>(getLast(m_normal[0].name()));
    m_metalness_roughnessName = std::make_shared<std::string>(getLast(m_roughness_metalness[0].name()));
    m_specularName = std::make_shared<std::string>(getLast(m_specular[0].name()));
    m_ambientOcclusionName = std::make_shared<std::string>(getLast(m_ambientOcclusion[0].name()));

    m_componentTypeDiffuse=std::make_tuple(Prisma::Component::TYPES::STRING,"Diffuse",m_diffuseName.get());
    m_componentTypeNormal=std::make_tuple(Prisma::Component::TYPES::STRING,"Normal",m_normalName.get());
    m_componentTypeMetalnessRoughness=std::make_tuple(Prisma::Component::TYPES::STRING,"Metalness-Roughness",m_metalness_roughnessName.get());
    m_componentTypeSpecular = std::make_tuple(Prisma::Component::TYPES::STRING, "Specular", m_specularName.get());
    m_componentTypeAmbientOcclusion = std::make_tuple(Prisma::Component::TYPES::STRING, "Diffuse", m_ambientOcclusionName.get());

    addGlobal(m_componentTypeDiffuse);
    addGlobal(m_componentTypeNormal);
    addGlobal(m_componentTypeMetalnessRoughness);
    addGlobal(m_componentTypeSpecular);
    addGlobal(m_componentTypeAmbientOcclusion);

    m_id = materialId;
    materialId++;
}

void Prisma::MaterialComponent::update() {

}

const std::vector <Prisma::Texture>& Prisma::MaterialComponent::normal() const {
    return m_normal;
}

void Prisma::MaterialComponent::roughness_metalness(std::vector <Prisma::Texture> roughness_metalness) {
    m_roughness_metalness=roughness_metalness;
    Prisma::CacheScene::getInstance().updateTextures(true);
}

void Prisma::MaterialComponent::specular(std::vector<Prisma::Texture> specular) {
    m_specular = specular;
}

const std::vector<Prisma::Texture>& Prisma::MaterialComponent::specular() const {
    return m_specular;
}

const std::vector <Prisma::Texture>& Prisma::MaterialComponent::roughness_metalness() const {
    return m_roughness_metalness;
}

void Prisma::MaterialComponent::ambientOcclusion(std::vector<Prisma::Texture> ambientOcclusion) {
    m_ambientOcclusion = ambientOcclusion;
}

const std::vector<Prisma::Texture>& Prisma::MaterialComponent::ambientOcclusion() const {
    return m_ambientOcclusion;
}

unsigned int Prisma::MaterialComponent::material_id()
{
    return m_id;
}

void Prisma::MaterialComponent::material_name(std::string name) {
    m_materialName = name;
}

Prisma::MaterialComponent::MaterialComponent() {
    name("Material Component");
}

std::string Prisma::MaterialComponent::material_name() {
    return m_materialName;
}

void Prisma::MaterialComponent::transparent(bool transparent) {
    m_transparent = transparent;
}

bool Prisma::MaterialComponent::transparent() {
    return m_transparent;
}
