#include "Pipelines/PipelineFullScreen.h"
#include "Helpers/PrismaRender.h"

Prisma::PipelineFullScreen::PipelineFullScreen() {
        /*m_shader = std::make_shared<Shader>("../../../Engine/Shaders/FullScreen/vertex.glsl",
                                            "../../../Engine/Shaders/FullScreen/fragment.glsl");
        m_bindlessPos = m_shader->getUniformPosition("screenTexture");*/
}

void Prisma::PipelineFullScreen::render(uint64_t texture) {
        /*m_shader->use();
        glDisable(GL_DEPTH_TEST);
        m_shader->setInt64(m_bindlessPos, texture);
        //PrismaRender::getInstance().renderQuad();
        glEnable(GL_DEPTH_TEST);*/
}