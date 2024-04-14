#include "../../include/Pipelines/PipelineSSR.h"
#include "../../include/Helpers/IBLBuilder.h"


Prisma::PipelineSSR::PipelineSSR() {
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/SSRPipeline/vertex.glsl",
                                        "../../../Engine/Shaders/SSRPipeline/fragment.glsl");

    m_shader->use();
    m_albedoPos=m_shader->getUniformPosition("textureFrame");
    m_normalPos=m_shader->getUniformPosition("textureNorm");
    m_metalnessPos=m_shader->getUniformPosition("textureMetallic");
    m_samplingPos=m_shader->getUniformPosition("samplingCoefficient");

}

void Prisma::PipelineSSR::update(uint64_t albedo, uint64_t metalness, uint64_t normal) {
    m_shader->use();
    m_shader->setInt64(m_albedoPos,albedo);
    m_shader->setInt64(m_normalPos,normal);
    m_shader->setInt64(m_metalnessPos,metalness);
    m_shader->setFloat(m_samplingPos,0.2);

    Prisma::IBLBuilder::getInstance().renderQuad();
}
