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
    m_fbo->bind();
    for (auto effect : m_effects) {
        effect->render(m_fbo->texture());
    }
    m_fbo->unbind();
}

void Prisma::Postprocess::addPostProcess(std::shared_ptr<Prisma::PostprocessEffect> postprocessEffect)
{
    m_effects.push_back(postprocessEffect);
}

void Prisma::Postprocess::fbo(std::shared_ptr<Prisma::FBO> fbo)
{
    m_fbo = fbo;
}
