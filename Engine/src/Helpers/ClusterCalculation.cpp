#include "../../include/Helpers/ClusterCalculation.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/GlobalData/GlobalData.h"
#include <glm/gtx/string_cast.hpp>

Prisma::ClusterCalculation::ClusterCalculation(unsigned int numClusters)
{
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/ComputePipeline/compute.glsl");
	m_shaderLights = std::make_shared<Shader>("../../../Engine/Shaders/ComputePipeline/compute_lights.glsl");
	m_ssbo = std::make_shared<SSBO>(5);
	m_ssbo->resize(numClusters * sizeof(Cluster), GL_STATIC_DRAW);
	m_shader->use();
	m_nearPos = m_shader->getUniformPosition("zNear");
	m_farPos = m_shader->getUniformPosition("zFar");
	m_inverseProjectionPos = m_shader->getUniformPosition("inverseProjection");
	m_gridSizePos = m_shader->getUniformPosition("gridSize");
	m_screenDimensionsPos = m_shader->getUniformPosition("screenDimensions");
}

void Prisma::ClusterCalculation::updateCamera()
{
	auto currentSettings = SettingsLoader::getInstance().getSettings();
	m_shader->use();
	m_shader->setFloat(m_nearPos, currentGlobalScene->camera->nearPlane());
	m_shader->setFloat(m_farPos, currentGlobalScene->camera->farPlane());
	m_shader->setMat4(m_inverseProjectionPos, glm::inverse(currentProjection));
	m_shader->setUVec3(m_gridSizePos, {m_gridSizeX, m_gridSizeY, m_gridSizeZ});
	m_shader->setUVec2(m_screenDimensionsPos, {currentSettings.width, currentSettings.height});
	m_shader->dispatchCompute({m_gridSizeX, m_gridSizeY, m_gridSizeZ});
	m_shader->wait(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Prisma::ClusterCalculation::updateLights()
{
	m_shaderLights->use();
	m_shaderLights->dispatchCompute({27, 1, 1});
	m_shaderLights->wait(GL_SHADER_STORAGE_BARRIER_BIT);
}
