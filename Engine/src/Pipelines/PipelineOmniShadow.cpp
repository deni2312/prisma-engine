#include "../../include/Pipelines/PipelineOmniShadow.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../../GUI/include/TextureInfo.h"

static std::shared_ptr<Prisma::Shader> m_shader = nullptr;
static std::shared_ptr<Prisma::Shader> m_shaderAnimation = nullptr;

Prisma::PipelineOmniShadow::PipelineOmniShadow(unsigned int width, unsigned int height):m_width{width},m_height{height} {

    if(!m_shader){
        m_shader=std::make_shared<Shader>("../../../Engine/Shaders/OmniShadowPipeline/vertex.glsl", "../../../Engine/Shaders/OmniShadowPipeline/fragment.glsl","../../../Engine/Shaders/OmniShadowPipeline/geometry.glsl");
        m_shaderAnimation = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_omni.glsl", "../../../Engine/Shaders/OmniShadowPipeline/fragment.glsl", "../../../Engine/Shaders/OmniShadowPipeline/geometry.glsl");
    }

    glGenFramebuffers(1, &m_fbo);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_shader->use();
    m_farPlanePos=m_shader->getUniformPosition("far_plane");
    m_lightPos=m_shader->getUniformPosition("lightPos");
    for (unsigned int i = 0; i < 6; ++i)
        m_shadowPosition.push_back(m_shader->getUniformPosition("shadowMatrices[" + std::to_string(i) + "]"));

    m_shaderAnimation->use();
    m_farPlanePosAnimation = m_shaderAnimation->getUniformPosition("far_plane");
    m_lightPosAnimation = m_shaderAnimation->getUniformPosition("lightPos");
    for (unsigned int i = 0; i < 6; ++i)
        m_shadowPositionAnimation.push_back(m_shaderAnimation->getUniformPosition("shadowMatrices[" + std::to_string(i) + "]"));
    m_id = glGetTextureHandleARB(depthCubemap);
    glMakeTextureHandleResidentARB(m_id);
}

void Prisma::PipelineOmniShadow::update(glm::vec3 lightPos) {
    m_shadowProj = glm::perspective(glm::radians(90.0f), (float)m_width / (float)m_height, m_nearPlane, m_farPlane);
    m_shadowTransforms.clear();
    m_shadowTransforms.push_back(m_shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    m_shadowTransforms.push_back(m_shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    m_shadowTransforms.push_back(m_shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    m_shadowTransforms.push_back(m_shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    m_shadowTransforms.push_back(m_shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    m_shadowTransforms.push_back(m_shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shader->use();
    for (unsigned int i = 0; i < 6; ++i) {
        m_shader->setMat4(m_shadowPosition[i], m_shadowTransforms[i]);
    }
    m_shader->setFloat(m_farPlanePos, m_farPlane);
    m_shader->setVec3(m_lightPos, lightPos);

    Prisma::MeshIndirect::getInstance().renderMeshes();

    m_shaderAnimation->use();

    for (unsigned int i = 0; i < 6; ++i) {
        m_shaderAnimation->setMat4(m_shadowPositionAnimation[i], m_shadowTransforms[i]);
    }
    m_shaderAnimation->setFloat(m_farPlanePosAnimation, m_farPlane);
    m_shaderAnimation->setVec3(m_lightPosAnimation, lightPos);

    Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // don't forget to configure the viewport to the capture dimensions.
}

uint64_t Prisma::PipelineOmniShadow::id() {
    return m_id;
}

float Prisma::PipelineOmniShadow::farPlane() {
    return m_farPlane;
}

void Prisma::PipelineOmniShadow::farPlane(float farPlane) {
    m_farPlane = farPlane;
}

float Prisma::PipelineOmniShadow::nearPlane() {
    return m_nearPlane;
}

void Prisma::PipelineOmniShadow::nearPlane(float nearPlane) {
    m_nearPlane = nearPlane;
}
