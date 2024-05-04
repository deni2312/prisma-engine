#include "../../include/Pipelines/PipelineCSM.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/GlobalData/GlobalData.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Helpers/SettingsLoader.h"
#include <glm/gtx/string_cast.hpp>


static std::shared_ptr<Prisma::Shader> m_shader = nullptr;
static std::shared_ptr<Prisma::Shader> m_shaderAnimation = nullptr;

Prisma::PipelineCSM::PipelineCSM(unsigned int width, unsigned int height) :m_width{ width }, m_height{ height } {

    if (!m_shader) {
        m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CSMPipeline/vertex.glsl", "../../../Engine/Shaders/CSMPipeline/fragment.glsl");
        m_shaderAnimation = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_CSM.glsl", "../../../Engine/Shaders/CSMPipeline/fragment.glsl");
    }

    glGenFramebuffers(1, &m_fbo);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_id = glGetTextureHandleARB(depthMap);
    glMakeTextureHandleResidentARB(m_id);

    m_shader->use();
    m_posLightmatrix = m_shader->getUniformPosition("lightSpaceMatrix");

    m_shaderAnimation->use();
    m_posLightmatrixAnimation = m_shaderAnimation->getUniformPosition("lightSpaceMatrix");
    m_projectionLength = glm::vec4(-10.0f, 10.0f, -10.0f, 10.0f);
}

void Prisma::PipelineCSM::update(glm::vec3 lightPos) {
    m_lightDir = lightPos;
    glm::mat4 lightProjection, lightView;
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    lightProjection = glm::ortho(m_projectionLength.x, m_projectionLength.y, m_projectionLength.z, m_projectionLength.w , m_nearPlane, m_farPlane);
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    m_lightSpaceMatrix = lightProjection * lightView;

    m_shader->use();
    
    m_shader->setMat4(m_posLightmatrix, m_lightSpaceMatrix);

    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    Prisma::MeshIndirect::getInstance().renderMeshes();

    m_shaderAnimation->use();

    m_shaderAnimation->setMat4(m_posLightmatrixAnimation, m_lightSpaceMatrix);

    Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // don't forget to configure the viewport to the capture dimensions.
}

glm::mat4 Prisma::PipelineCSM::lightMatrix() {
    return m_lightSpaceMatrix;
}

void Prisma::PipelineCSM::projectionLength(glm::vec4 projectionLength) {
    m_projectionLength = projectionLength;
    updateLights = true;
}

glm::vec4 Prisma::PipelineCSM::projectionLength() {
    return m_projectionLength;
}

uint64_t Prisma::PipelineCSM::id() {
    return m_id;
}

float Prisma::PipelineCSM::farPlane() {
    return m_farPlane;
}

void Prisma::PipelineCSM::farPlane(float farPlane)
{
    m_farPlane = farPlane;
}

float Prisma::PipelineCSM::nearPlane()
{
    return m_nearPlane;
}

void Prisma::PipelineCSM::nearPlane(float nearPlane)
{
    m_nearPlane = nearPlane;
}

void Prisma::PipelineCSM::lightDir(glm::vec3 lightDir) {
    m_lightDir = lightDir;
}

glm::vec3 Prisma::PipelineCSM::lightDir() {
    return m_lightDir;
}
