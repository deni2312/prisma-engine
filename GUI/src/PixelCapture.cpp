#include "../include/PixelCapture.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"

std::shared_ptr<Prisma::PixelCapture> Prisma::PixelCapture::instance = nullptr;

Prisma::PixelCapture::PixelCapture()
{

    auto settings = Prisma::SettingsLoader::instance().getSettings();

    Prisma::FBO::FBOData fboData;
    fboData.enableDepth = true;
    fboData.height = settings.height;
    fboData.width = settings.width;

    m_fbo = std::make_shared<Prisma::FBO>(fboData);

    m_shader = std::make_shared<Shader>("../../../GUI/Shaders/PixelCapture/vertex.glsl", "../../../GUI/Shaders/PixelCapture/fragment.glsl");
}

void Prisma::PixelCapture::capture()
{
    m_fbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();

    Prisma::MeshIndirect::getInstance().renderMeshes();
    m_fbo->unbind();
}

Prisma::PixelCapture& Prisma::PixelCapture::getInstance()
{
    if (!instance) {
        instance = std::make_shared<PixelCapture>();
    }
    return *instance;
}
