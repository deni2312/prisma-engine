#include "../include/PixelCapture.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Handlers/MeshHandler.h"

static std::shared_ptr<Prisma::Shader> shader = nullptr;
static std::shared_ptr<Prisma::Shader> shaderAnimation = nullptr;

Prisma::PixelCapture::PixelCapture()
{
    if (!shader) {
        auto settings = Prisma::SettingsLoader::getInstance().getSettings();

        Prisma::FBO::FBOData fboData;
        fboData.enableDepth = true;
        fboData.height = settings.height;
        fboData.width = settings.width;

        m_fbo = std::make_shared<Prisma::FBO>(fboData);

        shader = std::make_shared<Shader>("../../../GUI/Shaders/PixelCapture/vertex.glsl", "../../../GUI/Shaders/PixelCapture/fragment.glsl");

        shaderAnimation = std::make_shared<Shader>("../../../GUI/Shaders/PixelCapture/vertex_animation.glsl", "../../../GUI/Shaders/PixelCapture/fragment_animation.glsl");
    }
}

std::shared_ptr<Prisma::Mesh> Prisma::PixelCapture::capture(glm::vec2 position)
{
    m_fbo->bind();
    GLfloat bkColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor);
    
    float color = bkColor[3];

    bkColor[3] = 0.2;

    glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bkColor[3] = color;

    glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]);

    shader->use();

    Prisma::MeshIndirect::getInstance().renderMeshes();

    shaderAnimation->use();

    Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

    glFlush();
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char data[4];

    glReadPixels(position.x, position.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    m_fbo->unbind();

    uint32_t encodedUUID = (data[0] << 16) | (data[1] << 8) | data[2];
    if (data[3] < 0.1) {
        if (encodedUUID < currentGlobalScene->meshes.size() && encodedUUID >= 0) {
            return currentGlobalScene->meshes[encodedUUID];
        }
    }
    else if (data[3] < 255 && data[3] > 0) {
        return nullptr;
    }
    else {
        if (encodedUUID < currentGlobalScene->animateMeshes.size() && encodedUUID >= 0) {
            return currentGlobalScene->animateMeshes[encodedUUID];
        }
    }
    return nullptr;
}
