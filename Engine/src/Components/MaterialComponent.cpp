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

std::vector <Prisma::Texture> Prisma::MaterialComponent::diffuse() {
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
    m_componentTypeDiffuse=std::make_tuple(Prisma::Component::TYPES::STRING,"Diffuse",m_diffuseName.get());
    m_componentTypeNormal=std::make_tuple(Prisma::Component::TYPES::STRING,"Normal",m_normalName.get());
    m_componentTypeMetalnessRoughness=std::make_tuple(Prisma::Component::TYPES::STRING,"Metalness-Roughness",m_metalness_roughnessName.get());
    addGlobal(m_componentTypeDiffuse);
    addGlobal(m_componentTypeNormal);
    addGlobal(m_componentTypeMetalnessRoughness);
    m_id = materialId;
    materialId++;
}

void Prisma::MaterialComponent::update() {

}

std::vector <Prisma::Texture> Prisma::MaterialComponent::normal() {
    return m_normal;
}

void Prisma::MaterialComponent::roughness_metalness(std::vector <Prisma::Texture> roughness_metalness) {
    m_roughness_metalness=roughness_metalness;
    Prisma::CacheScene::getInstance().updateTextures(true);
}

std::vector <Prisma::Texture> Prisma::MaterialComponent::roughness_metalness() {
    return m_roughness_metalness;
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
