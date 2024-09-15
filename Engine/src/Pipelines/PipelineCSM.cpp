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
#include "../../../GUI/include/TextureInfo.h"
#include "../../include/Helpers/PrismaRender.h"

static uint64_t numCSM = 0;

Prisma::PipelineCSM::PipelineCSM(unsigned int width, unsigned int height) :m_width{ width }, m_height{ height } {
    m_numCSM = numCSM;
    numCSM++;
    m_settings = Prisma::SettingsLoader::getInstance().getSettings();
    if (!m_numCSM) {
        if (!m_shader) {
            m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CSMPipeline/vertex.glsl", "../../../Engine/Shaders/CSMPipeline/fragment.glsl", "../../../Engine/Shaders/CSMPipeline/geometry.glsl");
            m_shaderAnimation = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_CSM.glsl", "../../../Engine/Shaders/CSMPipeline/fragment.glsl", "../../../Engine/Shaders/CSMPipeline/geometry.glsl");
            m_ssbo = std::make_shared<Prisma::SSBO>(10);
            m_ssbo->resize(sizeof(glm::mat4) * 16);
        }

        m_shadowCascadeLevels = { m_farPlane / 50.0f, m_farPlane / 25.0f, m_farPlane / 10.0f, m_farPlane / 2.0f };

        glGenFramebuffers(1, &m_fbo);

        unsigned int lightDepthMaps;

        glGenTextures(1, &lightDepthMaps);
        glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);


        glTexImage3D(
            GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, width, height, int(m_shadowCascadeLevels.size()) + 1,
            0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMaps, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_id = glGetTextureHandleARB(lightDepthMaps);
        glMakeTextureHandleResidentARB(m_id);
    }
}

void Prisma::PipelineCSM::update(glm::vec3 lightPos) {
    if (!m_numCSM) {
        m_lightDir = lightPos;
        if (m_ssbo) {
            auto lightMatrices = getLightSpaceMatrices();

            m_ssbo->modifyData(0, lightMatrices.size() * sizeof(glm::mat4), lightMatrices.data());

            m_shader->use();

            GLint viewport[4];

            glGetIntegerv(GL_VIEWPORT, viewport);

            glViewport(0, 0, m_width, m_height);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glClear(GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);  // peter panning
            Prisma::MeshIndirect::getInstance().renderMeshes();

            m_shaderAnimation->use();

            Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

            glCullFace(GL_BACK);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // don't forget to configure the viewport to the capture dimensions.
        }
    }
}

std::vector<glm::vec4> Prisma::PipelineCSM::getFrustumCornersWorldSpace(const glm::mat4& projview)
{
    const auto inv = glm::inverse(projview);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

std::vector<glm::vec4> Prisma::PipelineCSM::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    return getFrustumCornersWorldSpace(proj * view);
}

glm::mat4 Prisma::PipelineCSM::getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
    const auto proj = glm::perspective(
        glm::radians(90.0f), (float)m_settings.width / (float)m_settings.height, nearPlane,
        farPlane);

    const auto corners = getFrustumCornersWorldSpace(proj, currentGlobalScene->camera->matrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }

    center /= corners.size();

    auto direction = center + m_lightDir;

    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f); // Define the world's up direction

    // Check if direction and worldUp are parallel
    if (glm::abs(glm::dot(direction, worldUp)) > 0.99f) {
        // If they are nearly parallel, adjust the worldUp vector
        worldUp = glm::vec3(0.0f, 0.0f, 1.0f); // Set a different perpendicular direction
    }

    auto lightViewMatrix = glm::lookAt(direction, center, worldUp);

    //Get the longest radius in world space
    float radius = glm::length(center - glm::vec3(corners[6]));
    for (unsigned int i = 0; i < 8; ++i)
    {
        float distance = glm::length(glm::vec3(corners[i]) - center);
        radius = glm::max(radius, distance);

    }
    radius = std::ceil(radius);

    //Create the AABB from the radius
    glm::vec3 maxOrtho = center + glm::vec3(radius);
    glm::vec3 minOrtho = center - glm::vec3(radius);

    //Get the AABB in light view space
    maxOrtho = glm::vec3(glm::vec4(maxOrtho, 1.0f));
    minOrtho = glm::vec3(glm::vec4(minOrtho, 1.0f));

    //Just checking when debugging to make sure the AABB is the same size
    GLfloat lengthofTemp = glm::length(maxOrtho - minOrtho);

    //Store the far and near planes
    float far = maxOrtho.z;
    float near = minOrtho.z;

    auto lightOrthoMatrix = glm::ortho(minOrtho.x, maxOrtho.x, minOrtho.y, maxOrtho.y, near, far);

    glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
    glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadowMatrix * shadowOrigin;
    float storedW = shadowOrigin.w;
    shadowOrigin = shadowOrigin * (float)m_width / 2.0f;

    glm::vec4 roundedOrigin = glm::round(shadowOrigin);
    glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
    roundOffset = roundOffset * 2.0f / (float)m_width;
    roundOffset.z = 0.0f;
    roundOffset.w = 0.0f;

    glm::mat4 shadowProj = lightOrthoMatrix;
    shadowProj[3] += roundOffset;
    lightOrthoMatrix = shadowProj;

    return lightOrthoMatrix * lightViewMatrix;
}

std::vector<glm::mat4> Prisma::PipelineCSM::getLightSpaceMatrices()
{
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < m_shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(m_nearPlane, m_shadowCascadeLevels[i]));
        }
        else if (i < m_shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(m_shadowCascadeLevels[i - 1], m_shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(m_shadowCascadeLevels[i - 1], m_farPlane));
        }
    }
    return ret;
}

std::vector<float>& Prisma::PipelineCSM::cascadeLevels() {
    return m_shadowCascadeLevels;
}

void Prisma::PipelineCSM::bias(float bias) {
    m_bias = bias;
}

float Prisma::PipelineCSM::bias() {
    return m_bias;
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
    m_shadowCascadeLevels = { m_farPlane / 50.0f, m_farPlane / 25.0f, m_farPlane / 10.0f, m_farPlane / 2.0f };
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