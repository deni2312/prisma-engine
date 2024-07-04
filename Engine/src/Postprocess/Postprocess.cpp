#include "../../include/Postprocess/Postprocess.h"

std::shared_ptr<Prisma::Postprocess> Prisma::Postprocess::instance = nullptr;

Prisma::Postprocess& Prisma::Postprocess::getInstance()
{
    if (!instance) {
        instance = std::make_shared<Prisma::Postprocess>();
    }
    return *instance;
}

Prisma::Postprocess::Postprocess()
{

}

void Prisma::Postprocess::render()
{
    if (m_fbo) {
        m_fbo->bind();
        for (auto effect : m_effects) {
            effect->render(m_fbo, m_fboRaw);
        }
        m_fbo->unbind();
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        for (auto effect : m_effects) {
            effect->render(m_fbo, m_fboRaw);
        }
    }
}

void Prisma::Postprocess::addPostProcess(std::shared_ptr<Prisma::PostprocessEffect> postprocessEffect)
{
    m_effects.push_back(postprocessEffect);
}

void Prisma::Postprocess::fbo(std::shared_ptr<Prisma::FBO> fbo)
{
    m_fbo = fbo;
}

void Prisma::Postprocess::fboRaw(std::shared_ptr<Prisma::FBO> fbo) {
    m_fboRaw = fbo;
}
