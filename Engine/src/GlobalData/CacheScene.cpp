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

bool Prisma::CacheScene::updateTextures() const {
    return m_updateTextures;
}

void Prisma::CacheScene::updateTextures(bool value) {
    m_updateTextures = value;
}

bool Prisma::CacheScene::updateStatus()
{
    return m_updateStatus;
}

void Prisma::CacheScene::updateStatus(bool value)
{
    m_updateStatus = value;
}

Prisma::CacheScene& Prisma::CacheScene::getInstance()
{
    if (!instance) {
        instance = std::make_shared<CacheScene>();
    }
    return *instance;
}

void Prisma::CacheScene::resetCaches() {
    m_updateLights = false;
    m_updateSizes = false;
    m_updateData = false;
    m_updateTextures = false;
    m_updateStatus = false;
}
