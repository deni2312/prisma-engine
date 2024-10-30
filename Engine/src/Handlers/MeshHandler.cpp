#include "../../include/Handlers/MeshHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include <glm/gtx/string_cast.hpp>

void Prisma::MeshHandler::updateCamera()
{
	auto cameraMatrix = currentGlobalScene->camera->matrix();
	getInstance().ubo()->modifyData(0, sizeof(glm::mat4), value_ptr(cameraMatrix));
}

void Prisma::MeshHandler::updateCluster()
{
	m_uboClusterData.zNear = currentGlobalScene->camera->nearPlane();
	m_uboClusterData.zFar = currentGlobalScene->camera->farPlane();
	m_uboClusterData.gridSize = glm::vec4(ClusterCalculation::grids(), 1.0f);
	m_uboClusterData.screenDimensions = {m_settings.width, m_settings.height, 1.0f, 1.0f};
	m_uboCluster->modifyData(0, sizeof(UBOCluster), &m_uboClusterData);
}

void Prisma::MeshHandler::updateFragment()
{
	m_fragment.irradiancePos = PipelineDiffuseIrradiance::getInstance().id();
	m_fragment.prefilterPos = PipelinePrefilter::getInstance().id();
	m_fragment.lutPos = PipelineLUT::getInstance().id();
	m_fragment.viewPos = glm::vec4(currentGlobalScene->camera->position(), 1.0f);
	m_uboFragment->modifyData(0, sizeof(UBOFragment), &m_fragment);
}

Prisma::MeshHandler::MeshHandler()
{
	m_ubo = std::make_shared<Ubo>(sizeof(UBOData), 1);
	m_uboCluster = std::make_shared<Ubo>(sizeof(UBOCluster), 2);
	m_uboFragment = std::make_shared<Ubo>(sizeof(UBOFragment), 3);
	m_uboData = std::make_shared<UBOData>();
	m_settings = SettingsLoader::getInstance().getSettings();
}

std::shared_ptr<Prisma::MeshHandler::UBOData> Prisma::MeshHandler::data() const
{
	return m_uboData;
}

std::shared_ptr<Prisma::Ubo> Prisma::MeshHandler::ubo() const
{
	return m_ubo;
}
