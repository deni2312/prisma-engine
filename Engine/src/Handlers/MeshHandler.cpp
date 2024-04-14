#include "../../include/Handlers/MeshHandler.h"
#include "../../include/GlobalData/GlobalData.h"

std::shared_ptr<Prisma::MeshHandler> Prisma::MeshHandler::instance = nullptr;

void Prisma::MeshHandler::updateCamera() {
	auto cameraMatrix = currentGlobalScene->camera->matrix();
	MeshHandler::getInstance().ubo()->modifyData(Prisma::MeshHandler::VIEW_OFFSET, sizeof(glm::mat4), glm::value_ptr(cameraMatrix));
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
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
	m_uboData = std::make_shared<Prisma::MeshHandler::UBOData>();
}

std::shared_ptr<Prisma::MeshHandler::UBOData> Prisma::MeshHandler::data() const
{
	return m_uboData;
}

std::shared_ptr<Prisma::Ubo> Prisma::MeshHandler::ubo() const
{
	return m_ubo;
}
