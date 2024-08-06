#include "../../include/Pipelines/PipelineSSR.h"
#include "../../include/Helpers/IBLBuilder.h"


Prisma::PipelineSSR::PipelineSSR() {
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/SSRPipeline/vertex.glsl",
                                        "../../../Engine/Shaders/SSRPipeline/fragment.glsl");

    m_shader->use();
    m_albedoPos=m_shader->getUniformPosition("textureAlbedo");
    m_normalPos=m_shader->getUniformPosition("textureNorm");
    m_positionPos=m_shader->getUniformPosition("texturePosition");
    m_samplingPos=m_shader->getUniformPosition("screenSize");
    m_finalImagePos = m_shader->getUniformPosition("finalImage");
    m_depthPos = m_shader->getUniformPosition("textureDepth");

}

void Prisma::PipelineSSR::update(uint64_t albedo, uint64_t position, uint64_t normal,uint64_t finalImage,uint64_t depth) {
    MeshHandler::getInstance().updateCamera();

    m_shader->use();
    m_shader->setInt64(m_albedoPos,albedo);
    m_shader->setInt64(m_normalPos,normal);
    m_shader->setInt64(m_positionPos,position);
    m_shader->setInt64(m_finalImagePos, finalImage);
    m_shader->setInt64(m_depthPos, depth);
    m_shader->setVec2(m_samplingPos,glm::vec2(1440,2560));

    Prisma::IBLBuilder::getInstance().renderQuad();
}
