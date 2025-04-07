#include "GlobalData/CacheScene.h"

bool Prisma::CacheScene::updateLights()
{
	return m_updateLights;
}

void Prisma::CacheScene::updateLights(bool value)
{
	m_updateLights = value;
}

bool Prisma::CacheScene::updateSizeLights()
{
	return m_updateSizeLights;
}

void Prisma::CacheScene::updateSizeLights(bool value)
{
	m_updateSizeLights = value;
}

bool Prisma::CacheScene::updateSizes() const
{
	return m_updateSizes;
}

void Prisma::CacheScene::updateSizes(bool value)
{
	m_updateSizes = value;
}

bool Prisma::CacheScene::updateData() const
{
	return m_updateData;
}

void Prisma::CacheScene::updateData(bool value)
{
	m_updateData = value;
}

bool Prisma::CacheScene::updateTextures() const
{
	return m_updateTextures;
}

void Prisma::CacheScene::updateTextures(bool value)
{
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

void Prisma::CacheScene::resetCaches()
{
	m_updateLights = false;
	m_updateSizeLights = false;
	m_updateSizes = false;
	m_updateData = false;
	m_updateTextures = false;
	m_updateStatus = false;
}

void Prisma::CacheScene::updateAllCaches()
{
	m_updateLights = true;
	m_updateSizeLights = true;
	m_updateSizes = true;
	m_updateData = true;
	m_updateTextures = true;
	m_updateStatus = true;
}
