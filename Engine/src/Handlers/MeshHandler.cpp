#include "../../include/Handlers/MeshHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/Helpers/ClusterCalculation.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/Pipelines/PipelineLUT.h"
#include "../../include/Helpers/AreaHandler.h"
#include <glm/gtx/string_cast.hpp>

#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

void Prisma::MeshHandler::updateCamera()
{
	auto camera = Prisma::GlobalData::getInstance().currentGlobalScene()->camera;
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
	Diligent::MapHelper<ViewProjectionData> viewProjection(contextData.m_pImmediateContext, m_viewProjection, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
	viewProjection->view = camera->matrix();
	viewProjection->projection = Prisma::GlobalData::getInstance().currentProjection();
	viewProjection->viewPos = glm::vec4(camera->position(),1.0);
	//getInstance().ubo()->modifyData(0, sizeof(glm::mat4), value_ptr(cameraMatrix));
}

void Prisma::MeshHandler::updateCluster()
{
	m_clusterData.zNear = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane();
	m_clusterData.zFar = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane();
	m_clusterData.gridSize = glm::vec4(ClusterCalculation::grids(), 1.0f);
	m_clusterData.screenDimensions = {m_settings.width, m_settings.height, 1.0f, 1.0f};
	//m_uboCluster->modifyData(0, sizeof(UBOCluster), &m_uboClusterData);
}

void Prisma::MeshHandler::updateFragment()
{
	m_fragment.irradiancePos = PipelineDiffuseIrradiance::getInstance().id();
	//m_fragment.prefilterPos = PipelinePrefilter::getInstance().id();
	//m_fragment.lutPos = PipelineLUT::getInstance().id();
	m_fragment.viewPos = glm::vec4(Prisma::GlobalData::getInstance().currentGlobalScene()->camera->position(), 1.0f);
	m_fragment.textureLut = Prisma::AreaHandler::getInstance().idLut();
	m_fragment.textureM = Prisma::AreaHandler::getInstance().idM();
	//m_uboFragment->modifyData(0, sizeof(UBOFragment), &m_fragment);
}

Prisma::MeshHandler::MeshHandler()
{
	m_settings = SettingsLoader::getInstance().getSettings();
	auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
	Diligent::BufferDesc CBDesc;
	CBDesc.Name = "View Projection";
	CBDesc.Size = sizeof(ViewProjectionData);
	CBDesc.Usage = Diligent::USAGE_DYNAMIC;
	CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
	CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
	contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_viewProjection);
}


Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::MeshHandler::viewProjection() const
{
	return m_viewProjection;
}
