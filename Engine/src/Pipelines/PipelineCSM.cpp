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


static std::shared_ptr<Prisma::Shader> shader = nullptr;
static std::shared_ptr<Prisma::Shader> shaderAnimation = nullptr;



Prisma::PipelineCSM::PipelineCSM(unsigned int width, unsigned int height) :m_width{ width }, m_height{ height } {

    if (!shader) {
        shader = std::make_shared<Shader>("../../../Engine/Shaders/CSMPipeline/vertex.glsl", "../../../Engine/Shaders/CSMPipeline/fragment.glsl", "../../../Engine/Shaders/CSMPipeline/geometry.glsl");
        shaderAnimation = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_CSM.glsl", "../../../Engine/Shaders/CSMPipeline/fragment.glsl", "../../../Engine/Shaders/CSMPipeline/geometry.glsl");
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

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

    shader->use();

    shaderAnimation->use();

}

void Prisma::PipelineCSM::update(glm::vec3 lightPos) {
    m_lightDir = lightPos;
    if (m_ssbo) {
        auto lightMatrices = getLightSpaceMatrices();

        m_ssbo->modifyData(0, lightMatrices.size() * sizeof(glm::mat4), lightMatrices.data());

        shader->use();

        GLint viewport[4];

        glGetIntegerv(GL_VIEWPORT, viewport);

        glViewport(0, 0, m_width, m_height);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);  // peter panning
        Prisma::MeshIndirect::getInstance().renderMeshes();

        shaderAnimation->use();

        Prisma::MeshIndirect::getInstance().renderAnimateMeshes();

        glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]); // don't forget to configure the viewport to the capture dimensions.
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
        glm::radians(m_settings.angle), (float)m_settings.width / (float)m_settings.height, nearPlane,
        farPlane);
    const auto corners = getFrustumCornersWorldSpace(proj, currentGlobalScene->camera->matrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    auto direction = center + m_lightDir;

    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::dot(direction, worldUp) > 0.99f) {
        worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::vec3 right = glm::normalize(glm::cross(worldUp, direction));
    glm::vec3 up = glm::cross(direction, right);

    glm::mat4 lightView = glm::lookAt(direction, center, up);

    // Initialize min and max values
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Align the orthographic projection to the shadow map resolution grid
    float worldUnitsPerTexel = (maxX - minX) / static_cast<float>(m_width);
    minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
    maxX = std::floor(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
    minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
    maxY = std::floor(maxY / worldUnitsPerTexel) * worldUnitsPerTexel;

    // Using a constant depth range for stability
    constexpr float zMult = 10.0f;
    minZ = -farPlane * zMult;
    maxZ = farPlane * zMult;

    glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
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
