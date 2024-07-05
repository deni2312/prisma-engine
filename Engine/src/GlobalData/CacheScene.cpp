#include "../../include/GlobalData/CacheScene.h"

std::shared_ptr<Prisma::CacheScene> Prisma::CacheScene::instance = nullptr;

bool Prisma::CacheScene::updateLights() {
    return m_updateLights;
}

void Prisma::CacheScene::updateLights(bool value) {
    m_updateLights = value;
}

bool Prisma::CacheScene::updateSizes() const {
    return m_updateSizes;
}

void Prisma::CacheScene::updateSizes(bool value) {
    m_updateSizes = value;
}

bool Prisma::CacheScene::updateData() const {
    return m_updateData;
}

void Prisma::CacheScene::updateData(bool value) {
    m_updateData = value;
}

bool Prisma::CacheScene::skipUpdate() const {
    return m_skipUpdate;
}

void Prisma::CacheScene::skipUpdate(bool value) {
    m_skipUpdate = value;
}

bool Prisma::CacheScene::updateTextures() const {
    return m_updateTextures;
}

void Prisma::CacheScene::updateTextures(bool value) {
    m_updateTextures = value;
}

Prisma::CacheScene& Prisma::CacheScene::getInstance()
{
    if (!instance) {
        instance = std::make_shared<CacheScene>();
    }
    return *instance;
}

void Prisma::CacheScene::resetCaches() {
    if (!m_skipUpdate) {
        m_updateLights = false;
        m_updateSizes = false;
        m_updateData = false;
        m_updateTextures = false;
    }
    else {
        m_skipUpdate = false;
    }
}
