#include "Pipelines/PipelineCSM.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../../../GUI/include/TextureInfo.h"
#include "GlobalData/GlobalData.h"
#include "Handlers/CSMHandler.h"
#include "Helpers/PrismaRender.h"
#include "Helpers/SettingsLoader.h"
#include "SceneData/MeshIndirect.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
//
// static std::shared_ptr<Prisma::Shader> shader = nullptr;
//
// static std::shared_ptr<Prisma::Shader> shaderAnimation = nullptr;
//
// static std::shared_ptr<Prisma::SSBO> ssbo = nullptr;

Prisma::PipelineCSM::PipelineCSM(unsigned int width, unsigned int height, bool post) : m_width{width}, m_height{height} {
    if (!post) {
        init();
    }
}

void Prisma::PipelineCSM::update(glm::vec3 lightPos) {
    m_lightDir = lightPos;
    createLightSpaceMatrices();
    m_lightMatrices.farPlaneCSM = m_farPlane;
    m_lightMatrices.sizeCSM = m_size - 1;
    m_lightMatrices.resolutionCSM = glm::vec2(m_width, m_height);
    CSMHandler::getInstance().render({m_depth, m_lightMatrices});
    // if (ssbo)
    //{
    /*auto lightMatrices = getLightSpaceMatrices();

    ssbo->modifyData(0, lightMatrices.size() * sizeof(glm::mat4), lightMatrices.data());

    shader->use();

    glViewport(0, 0, m_width, m_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT); // peter panning
    MeshIndirect::getInstance().renderMeshesCopy();

    shaderAnimation->use();

    MeshIndirect::getInstance().renderAnimateMeshes();

    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Prisma::SettingsLoader().getInstance().getSettings().width,
            Prisma::SettingsLoader().getInstance().getSettings().height);
    // don't forget to configure the viewport to the capture dimensions.*/
    //}
}

std::vector<glm::vec4> Prisma::PipelineCSM::getFrustumCornersWorldSpace(const glm::mat4& projview) {
    const auto inv = inverse(projview);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
            for (unsigned int z = 0; z < 2; ++z) {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::PipelineCSM::shadowTexture() { return m_depth; }

glm::mat4 Prisma::PipelineCSM::getLightSpaceMatrix(const float nearPlane, const float farPlane) {
    auto proj = glm::perspective(glm::radians(GlobalData::getInstance().currentGlobalScene()->camera->angle()), static_cast<float>(m_settings.width) / static_cast<float>(m_settings.height), nearPlane, farPlane);

    const auto corners = getFrustumCornersWorldSpace(proj * GlobalData::getInstance().currentGlobalScene()->camera->matrix());

    auto center = glm::vec3(0, 0, 0);
    for (const auto& v : corners) {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + m_lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners) {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0) {
        minZ *= zMult;
    } else {
        minZ /= zMult;
    }
    if (maxZ < 0) {
        maxZ /= zMult;
    } else {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
}

void Prisma::PipelineCSM::createLightSpaceMatrices() {
    for (size_t i = 0; i < m_size; ++i) {
        if (i == 0) {
            m_lightMatrices.shadows[i] = getLightSpaceMatrix(m_nearPlane, m_lightMatrices.cascadePlanes[i].x);
        } else if (i < m_size - 1) {
            m_lightMatrices.shadows[i] = getLightSpaceMatrix(m_lightMatrices.cascadePlanes[i - 1].x, m_lightMatrices.cascadePlanes[i].x);
        } else {
            m_lightMatrices.shadows[i] = getLightSpaceMatrix(m_lightMatrices.cascadePlanes[i - 1].x, m_farPlane);
        }
    }
}

void Prisma::PipelineCSM::bias(float bias) {
    m_bias = bias;
    CacheScene::getInstance().updateStatus(true);
}

float Prisma::PipelineCSM::bias() { return m_bias; }

float Prisma::PipelineCSM::farPlane() { return m_farPlane; }

void Prisma::PipelineCSM::farPlane(float farPlane) {
    m_farPlane = farPlane;
    m_lightMatrices.cascadePlanes[0].x = m_farPlane / 50.0f;
    m_lightMatrices.cascadePlanes[1].x = m_farPlane / 25.0f;
    m_lightMatrices.cascadePlanes[2].x = m_farPlane / 10.0f;
    m_lightMatrices.cascadePlanes[3].x = m_farPlane / 2.0f;
    CacheScene::getInstance().updateStatus(true);
}

void Prisma::PipelineCSM::init() {
    /*m_settings = SettingsLoader::getInstance().getSettings();

    if (!shader)
    {
            shader = std::make_shared<Shader>("../../../Engine/Shaders/CSMPipeline/vertex.glsl",
                    "../../../Engine/Shaders/CSMPipeline/fragment.glsl",
                    "../../../Engine/Shaders/CSMPipeline/geometry.glsl");
            shaderAnimation = std::make_shared<Shader>("../../../Engine/Shaders/AnimationPipeline/vertex_CSM.glsl",
                    "../../../Engine/Shaders/CSMPipeline/fragment.glsl",
                    "../../../Engine/Shaders/CSMPipeline/geometry.glsl");
            ssbo = std::make_shared<SSBO>(10);
            ssbo->resize(sizeof(glm::mat4) * 16);
    }

    m_shadowCascadeLevels = { m_farPlane / 50.0f, m_farPlane / 25.0f, m_farPlane / 10.0f, m_farPlane / 2.0f };

    glGenFramebuffers(1, &m_fbo);

    unsigned int lightDepthMaps;

    glGenTextures(1, &lightDepthMaps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMaps);


    glTexImage3D(
            GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, m_width, m_height,
            static_cast<int>(m_shadowCascadeLevels.size()) + 1,
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
    glMakeTextureHandleResidentARB(m_id);*/
    if (!m_init) {
        auto& contextData = PrismaFunc::getInstance().contextData();
        farPlane(m_farPlane);

        // Create window-size depth buffer
        Diligent::TextureDesc RTDepthDesc;
        RTDepthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
        RTDepthDesc.Width = m_width;
        RTDepthDesc.Height = m_height;
        RTDepthDesc.MipLevels = 1;
        RTDepthDesc.ArraySize = m_size;
        RTDepthDesc.Name = "Offscreen depth buffer CSM";
        RTDepthDesc.Format = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
        RTDepthDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
        // Define optimal clear value
        RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
        RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
        RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
        contextData.device->CreateTexture(RTDepthDesc, nullptr, &m_depth);

        // Create render target views for each face
        for (int i = 0; i < m_size; ++i) {
            Diligent::TextureViewDesc DepthDesc;
            DepthDesc.ViewType = Diligent::TEXTURE_VIEW_DEPTH_STENCIL;
            DepthDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
            DepthDesc.MostDetailedMip = 0;
            DepthDesc.NumMipLevels = 1;
            DepthDesc.FirstArraySlice = i;  // Select the specific face
            DepthDesc.NumArraySlices = 1;
            Diligent::RefCntAutoPtr<Diligent::ITextureView> depth;
            m_depth->CreateView(DepthDesc, &depth);
            Diligent::RefCntAutoPtr<Diligent::ITextureView> color;
        }
    }
    m_init = true;
}

float Prisma::PipelineCSM::nearPlane() { return m_nearPlane; }

void Prisma::PipelineCSM::nearPlane(float nearPlane) {
    m_nearPlane = nearPlane;
    CacheScene::getInstance().updateStatus(true);
}

void Prisma::PipelineCSM::lightDir(glm::vec3 lightDir) {
    m_lightDir = lightDir;
    CacheScene::getInstance().updateLights();
}

glm::vec3 Prisma::PipelineCSM::lightDir() { return m_lightDir; }