#include "../include/PixelCapture.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Engine/include/GlobalData/GlobalData.h"

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

std::shared_ptr<Prisma::Mesh> Prisma::PixelCapture::capture(glm::vec2 position)
{
    auto settings = Prisma::SettingsLoader::instance().getSettings();

    m_fbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();

    Prisma::MeshIndirect::getInstance().renderMeshes();

    glFlush();
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char data[4];

    glReadPixels(position.x, position.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    m_fbo->unbind();

    int meshId = data[0];

    std::cout << meshId<<std::endl;

    if (meshId < currentGlobalScene->meshes.size()) {
        std::cout << currentGlobalScene->meshes[meshId]->name() << std::endl;
    }

    return std::make_shared<Mesh>();
}

Prisma::PixelCapture& Prisma::PixelCapture::getInstance()
{
    if (!instance) {
        instance = std::make_shared<PixelCapture>();
    }
    return *instance;
}
