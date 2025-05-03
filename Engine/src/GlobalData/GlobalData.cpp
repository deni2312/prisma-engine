#include "GlobalData/GlobalData.h"

namespace Prisma {
class TextureInfo;
}

std::shared_ptr<Prisma::Scene> Prisma::GlobalData::currentGlobalScene() const {
    return m_currentGlobalScene;
}

void Prisma::GlobalData::currentGlobalScene(const std::shared_ptr<Scene>& currentGlobalScene) {
    m_currentGlobalScene = currentGlobalScene;
}

//std::shared_ptr<Prisma::FBO> Prisma::GlobalData::fboTarget() const
//{
//	return m_fboTarget;
//}

//void Prisma::GlobalData::fboTarget(const std::shared_ptr<FBO>& fboTarget)
//{
//	m_fboTarget = fboTarget;
//}

std::unordered_map<uint64_t, Prisma::Component*>& Prisma::GlobalData::sceneComponents() {
    return m_sceneComponents;
}

void Prisma::GlobalData::sceneComponents(const std::unordered_map<uint64_t, Component*>& sceneComponents) {
    m_sceneComponents = sceneComponents;
}

std::unordered_map<uint64_t, std::shared_ptr<Prisma::Node>>& Prisma::GlobalData::sceneNodes() {
    return m_sceneNodes;
}

void Prisma::GlobalData::sceneNodes(const std::unordered_map<uint64_t, std::shared_ptr<Node>>& sceneNodes) {
    m_sceneNodes = sceneNodes;
}

glm::mat4 Prisma::GlobalData::currentProjection() {
    return m_currentProjection;
}

void Prisma::GlobalData::currentProjection(const glm::mat4& currentProjection) {
    m_currentProjection = currentProjection;
}

Prisma::Texture& Prisma::GlobalData::defaultBlack() {
    return m_defaultBlack;
}

void Prisma::GlobalData::defaultBlack(const Texture& defaultBlack) {
    m_defaultBlack = defaultBlack;
}

Prisma::Texture& Prisma::GlobalData::defaultWhite() {
    return m_defaultWhite;
}

void Prisma::GlobalData::defaultWhite(const Texture& defaultWhite) {
    m_defaultWhite = defaultWhite;
}

Prisma::Texture& Prisma::GlobalData::defaultRoughness() { return m_defaultRoughness; }

void Prisma::GlobalData::defaultRoughness(const Texture& defaultRoughness) { m_defaultRoughness = defaultRoughness; }

Prisma::Texture& Prisma::GlobalData::defaultNormal() {
    return m_defaultNormal;
}

void Prisma::GlobalData::defaultNormal(const Texture& defaultNormal) {
    m_defaultNormal = defaultNormal;
}

void Prisma::GlobalData::transparencies(bool transparencies) {
    m_transparencies = transparencies;
}

bool Prisma::GlobalData::transparencies() const {
    return m_transparencies;
}

void Prisma::GlobalData::addGlobalTexture(GlobalTextureInfo texture) {
    m_textures.push_back(texture);
}

const std::vector<Prisma::GlobalData::GlobalTextureInfo>& Prisma::GlobalData::globalTextures() {
    return m_textures;
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::GlobalData::dummyTexture() {
    return m_pDummySRV;
}

void Prisma::GlobalData::dummyTexture(Diligent::RefCntAutoPtr<Diligent::ITexture> dummy) {
    m_pDummySRV = dummy;
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::GlobalData::dummyTextureArray() {
    return m_pDummyArraySRV;
}

void Prisma::GlobalData::dummyTextureArray(Diligent::RefCntAutoPtr<Diligent::ITexture> dummy) {
    m_pDummyArraySRV = dummy;
}