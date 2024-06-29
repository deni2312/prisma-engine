#include "../include/PixelCapture.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Handlers/MeshHandler.h"

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

    m_shaderAnimation = std::make_shared<Shader>("../../../GUI/Shaders/PixelCapture/vertex_animation.glsl", "../../../GUI/Shaders/PixelCapture/fragment_animation.glsl");

    m_shader->use();
}

std::shared_ptr<Prisma::Mesh> Prisma::PixelCapture::capture(glm::vec2 position)
{
    Prisma::MeshHandler::getInstance().updateCamera();
    m_fbo->bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();

    Prisma::MeshIndirect::getInstance().renderMeshes();

    m_shaderAnimation->use();

    Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

    glFlush();
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char data[4];

    glReadPixels(position.x, position.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    m_fbo->unbind();

    uint32_t encodedUUID = (data[0] << 16) | (data[1] << 8) | data[2];

    if (data[3] == 0) {
        if (encodedUUID < currentGlobalScene->meshes.size() && encodedUUID >= 0) {
            return currentGlobalScene->meshes[encodedUUID];
        }
    }
    else {
        if (encodedUUID < currentGlobalScene->animateMeshes.size() && encodedUUID >= 0) {
            return currentGlobalScene->animateMeshes[encodedUUID];
        }
    }
    return nullptr;
}

Prisma::PixelCapture& Prisma::PixelCapture::getInstance()
{
    if (!instance) {
        instance = std::make_shared<PixelCapture>();
    }
    return *instance;
}
