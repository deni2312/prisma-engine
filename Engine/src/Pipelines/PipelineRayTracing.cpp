#include "Pipelines/PipelineRayTracing.h"

#include "GlobalData/GlobalData.h"
#include "Helpers/PrismaRender.h"
#include "SceneData/MeshIndirect.h"
#include "SceneObjects/Mesh.h"
#include "Pipelines/PipelineSkybox.h"
#include "Pipelines/PipelineDIffuseIrradiance.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineLUT.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <memory>
#include <iostream>
#include "Helpers/SettingsLoader.h"
#include "Helpers/ClusterCalculation.h"
#include <random>
#include "Postprocess/Postprocess.h"
#include "Handlers/ComponentsHandler.h"

#include "Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h"
#include "Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h"
#include "Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h"
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"

#include "Common/interface/RefCntAutoPtr.hpp"

#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>

#include "Handlers/LightHandler.h"
#include "Pipelines/PipelineHandler.h"
#include "GlobalData/GlobalShaderNames.h"
#include <string>

using namespace Diligent;

Prisma::PipelineRayTracing::PipelineRayTracing(const unsigned int& width, const unsigned int& height, bool srgb) : m_width{
                                                                                                                 width
}, m_height{ height }
{

}

void Prisma::PipelineRayTracing::render() {

}

Prisma::PipelineRayTracing::~PipelineRayTracing()
{
}
