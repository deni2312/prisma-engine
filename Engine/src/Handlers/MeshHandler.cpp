#include "../../include/Handlers/MeshHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"

std::shared_ptr<Prisma::MeshHandler> Prisma::MeshHandler::instance = nullptr;

void Prisma::MeshHandler::updateCamera() {
	auto cameraMatrix = currentGlobalScene->camera->matrix();
	MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::VIEW_OFFSET, sizeof(glm::mat4), glm::value_ptr(cameraMatrix));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void Prisma::MeshHandler::updateCluster()
{
	m_uboClusterData.zNear = m_settings.nearPlane;
	m_uboClusterData.zFar = m_settings.farPlane;
	m_uboClusterData.gridSize = glm::vec4(Prisma::ClusterCalculation::grids(),1.0f);
	m_uboClusterData.screenDimensions = { m_settings.width,m_settings.height,1.0f,1.0f };
	m_uboCluster->modifyData(0, sizeof(Prisma::MeshHandler::UBOCluster), &m_uboClusterData);
}

Prisma::MeshHandler& Prisma::MeshHandler::getInstance()
{
	if (!instance) {
		instance = std::make_shared<MeshHandler>();
	}
	return *instance;
}

Prisma::MeshHandler::MeshHandler()
{
	m_ubo = std::make_shared<Ubo>(sizeof(Prisma::MeshHandler::UBOData), 1);
	m_uboCluster = std::make_shared<Ubo>(sizeof(Prisma::MeshHandler::UBOCluster), 2);
	m_uboData = std::make_shared<Prisma::MeshHandler::UBOData>();
	m_settings = Prisma::SettingsLoader::instance().getSettings();
}

std::shared_ptr<Prisma::MeshHandler::UBOData> Prisma::MeshHandler::data() const
{
	return m_uboData;
}

std::shared_ptr<Prisma::Ubo> Prisma::MeshHandler::ubo() const
{
	return m_ubo;
}
