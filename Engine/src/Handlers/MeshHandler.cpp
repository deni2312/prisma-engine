#include "Handlers/MeshHandler.h"
#include "GlobalData/GlobalData.h"
#include "Helpers/SettingsLoader.h"
#include "Helpers/ClusterCalculation.h"
#include "Pipelines/PipelineDIffuseIrradiance.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineLUT.h"
#include "Helpers/AreaHandler.h"
#include <glm/gtx/string_cast.hpp>

#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

void Prisma::MeshHandler::updateCamera() {
        auto camera = GlobalData::getInstance().currentGlobalScene()->camera;
        auto& contextData = PrismaFunc::getInstance().contextData();
        Diligent::MapHelper<ViewProjectionData> viewProjection(contextData.m_pImmediateContext, m_viewProjection,
                                                               Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        viewProjection->view = camera->matrix();
        viewProjection->projection = GlobalData::getInstance().currentProjection();
        viewProjection->viewPos = glm::vec4(camera->position(), 1.0);
}

void Prisma::MeshHandler::updateCluster() {
        /*m_clusterData.zNear = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->nearPlane();
        m_clusterData.zFar = Prisma::GlobalData::getInstance().currentGlobalScene()->camera->farPlane();
        m_clusterData.gridSize = glm::vec4(ClusterCalculation::grids(), 1.0f);
        m_clusterData.screenDimensions = {m_settings.width, m_settings.height, 1.0f, 1.0f};*/
        //m_uboCluster->modifyData(0, sizeof(UBOCluster), &m_uboClusterData);
}

Prisma::MeshHandler::MeshHandler() {
        m_settings = SettingsLoader::getInstance().getSettings();
        auto& contextData = PrismaFunc::getInstance().contextData();
        Diligent::BufferDesc CBDesc;
        CBDesc.Name = "View Projection";
        CBDesc.Size = sizeof(ViewProjectionData);
        CBDesc.Usage = Diligent::USAGE_DYNAMIC;
        CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_viewProjection);
}


Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::MeshHandler::viewProjection() const {
        return m_viewProjection;
}