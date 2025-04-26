#include "Pipelines/PipelineSSR.h"
#include "Helpers/PrismaRender.h"
#include "Helpers/SettingsLoader.h"
#include "engine.h"

Prisma::PipelineSSR::PipelineSSR() {
        //m_shader = std::make_shared<Shader>("../../../Engine/Shaders/SSRPipeline/vertex.glsl",
        //                                    "../../../Engine/Shaders/SSRPipeline/fragment.glsl");

        //m_shader->use();
        //m_albedoPos = m_shader->getUniformPosition("textureAlbedo");
        //m_normalPos = m_shader->getUniformPosition("textureNorm");
        //m_positionPos = m_shader->getUniformPosition("texturePosition");
        //m_finalImagePos = m_shader->getUniformPosition("finalImage");
        //m_depthPos = m_shader->getUniformPosition("textureDepth");
        //m_invProjPos = m_shader->getUniformPosition("invProjection");
        //auto settings = SettingsLoader::getInstance().getSettings();

        //FBO::FBOData fboData;
        //fboData.width = settings.width;
        //fboData.height = settings.height;
        //fboData.enableDepth = true;
        //fboData.internalFormat = GL_RGBA16F;
        //fboData.internalType = GL_FLOAT;
        //fboData.name = "SCREEN_SPACE_REFLECTIONS";
        //m_fboSSR = std::make_shared<FBO>(fboData);
}

void Prisma::PipelineSSR::update(uint64_t albedo, uint64_t position, uint64_t normal, uint64_t finalImage,
                                 uint64_t depth) {
        //m_fboSSR->bind();
        //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        //m_shader->use();
        //m_shader->setInt64(m_albedoPos, albedo);
        //m_shader->setInt64(m_normalPos, normal);
        //m_shader->setInt64(m_positionPos, position);
        //m_shader->setInt64(m_finalImagePos, finalImage);
        //m_shader->setInt64(m_depthPos, depth);
        //m_shader->setMat4(m_invProjPos, glm::inverse(Prisma::GlobalData::getInstance().currentProjection()));

        ////////PrismaRender::getInstance().renderQuad();
        //m_fboSSR->unbind();
}

//std::shared_ptr<Prisma::FBO> Prisma::PipelineSSR::texture()
//{
//	return m_fboSSR;
//}