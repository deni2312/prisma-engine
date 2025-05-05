#include "Handlers/LightHandler.h"
#include "GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>

#include "engine.h"
#include "GlobalData/CacheScene.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Helpers/Logger.h"


Prisma::LightHandler::LightHandler() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::BufferDesc OmniDesc;
    OmniDesc.Name = "Omni Light Buffer";
    OmniDesc.Usage = Diligent::USAGE_DEFAULT;
    OmniDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
    OmniDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    OmniDesc.ElementByteStride = sizeof(LightType::LightOmni);
    OmniDesc.Size = Define::MAX_OMNI_LIGHTS * sizeof(LightType::LightOmni);
    contextData.device->CreateBuffer(OmniDesc, nullptr, &m_omniLights);

    Diligent::BufferDesc DirDesc;
    DirDesc.Name = "Dir Light Buffer";
    DirDesc.Usage = Diligent::USAGE_DEFAULT;
    DirDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    DirDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    DirDesc.ElementByteStride = sizeof(LightType::LightDir);
    DirDesc.Size = Define::MAX_DIR_LIGHTS * sizeof(LightType::LightDir);
    contextData.device->CreateBuffer(DirDesc, nullptr, &m_dirLights);

    Diligent::BufferDesc LightSizeDesc;
    LightSizeDesc.Name = "Light sizes";
    LightSizeDesc.Usage = Diligent::USAGE_DEFAULT;
    LightSizeDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    LightSizeDesc.Size = sizeof(LightSizes);
    contextData.device->CreateBuffer(LightSizeDesc, nullptr, &m_lightSizes);
    m_clusterCalculation = std::make_shared<ClusterCalculation>(m_omniLights, m_lightSizes);

    m_init = true;
}

void Prisma::LightHandler::updateDirectional() {
    const auto& scene = GlobalData::getInstance().currentGlobalScene();
    int numVisible = 0;

    m_dataDirectional = std::make_shared<SSBODataDirectional>();
    for (int i = 0; i < scene->dirLights.size(); i++) {
        if (scene->dirLights[i]->visible()) {
            const auto& light = scene->dirLights[i];
            m_dataDirectional->lights.push_back(scene->dirLights[i]->type());
            const auto& dirMatrix = scene->dirLights[i]->finalMatrix();
            auto shadow = std::dynamic_pointer_cast<PipelineCSM>(light->shadow());
            const auto& dirMult = normalize(dirMatrix * m_dataDirectional->lights[i].direction);
            m_dataDirectional->lights[i].diffuse =
                m_dataDirectional->lights[i].diffuse * light->intensity();
            m_dataDirectional->lights[i].direction = dirMult;
            m_dataDirectional->lights[i].hasShadow = scene->dirLights[i]->hasShadow() ? 2.0f : 0.0f;
            m_dataDirectional->lights[i].bias = shadow->bias();
            numVisible++;
        }
    }
    m_sizes.dir = numVisible;

    if (!m_dataDirectional->lights.empty()) {
        auto& contextData = PrismaFunc::getInstance().contextData();
        contextData.immediateContext->UpdateBuffer(m_dirLights, 0, numVisible * sizeof(LightType::LightDir),
                                                   m_dataDirectional->lights.data(),
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}

void Prisma::LightHandler::updateArea() {
    const auto& scene = GlobalData::getInstance().currentGlobalScene();

    m_dataArea = std::make_shared<SSBODataArea>();
    int numVisible = 0;
    for (int i = 0; i < scene->areaLights.size(); i++) {
        const auto& light = scene->areaLights[i];
        if (light->visible()) {
            m_dataArea->lights.push_back(light->type());
            glm::mat4 areaMatrix;
            if (light->parent()) {
                areaMatrix = light->parent()->finalMatrix();
            } else {
                areaMatrix = light->matrix();
            }
            for (int j = 0; j < 4; j++) {
                m_dataArea->lights[i].position[j] = areaMatrix * m_dataArea->lights[i].position[j];
            }
            m_dataArea->lights[i].diffuse = m_dataArea->lights[i].diffuse * light->intensity();

            numVisible++;
        }
    }

    glm::ivec4 areaLength;
    areaLength.r = numVisible;
    //m_areaLights->modifyData(0, sizeof(glm::vec4),value_ptr(areaLength));
    //m_areaLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(LightType::LightArea),m_dataArea->lights.data());
}

void Prisma::LightHandler::updateOmni() {
    const auto& scene = GlobalData::getInstance().currentGlobalScene();

    if (m_init || CacheScene::getInstance().updateLights() || CacheScene::getInstance().updateStatus() ||
        CacheScene::getInstance().updateSizeLights()) {
        m_omniData.clear();
        m_dataOmni = std::make_shared<SSBODataOmni>();
        int numVisible = 0;
        int numShadow = 0;
        for (int i = 0; i < scene->omniLights.size(); i++) {
            const auto& light = scene->omniLights[i];
            if (light->visible()) {
                m_dataOmni->lights.push_back(light->type());
                glm::mat4 omniMatrix;
                if (light->parent()) {
                    omniMatrix = light->parent()->finalMatrix();
                } else {
                    omniMatrix = light->matrix();
                }
                m_dataOmni->lights[i].position = omniMatrix * m_dataOmni->lights[i].position;
                if (light->shadow() && light->hasShadow()) {
                    light->shadow()->update(m_dataOmni->lights[i].position);
                    m_dataOmni->lights[i].farPlane.x = light->shadow()->farPlane();
                    m_dataOmni->lights[i].shadowIndex = numShadow;
                    m_omniData.push_back(
                        light->shadow()->shadowTexture()->GetDefaultView(
                            Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
                    numShadow++;
                }
                m_dataOmni->lights[i].hasShadow = light->hasShadow() ? 2.0f : 0.0f;
                m_dataOmni->lights[i].diffuse = m_dataOmni->lights[i].diffuse * light->intensity();
                numVisible++;
            }
        }

        m_sizes.omni = numVisible;

        if (!m_dataOmni->lights.empty()) {
            auto& contextData = PrismaFunc::getInstance().contextData();
            contextData.immediateContext->UpdateBuffer(m_omniLights, 0,
                                                       numVisible * sizeof(LightType::LightOmni),
                                                       m_dataOmni->lights.data(),
                                                       Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
        for (auto update : m_updates) {
            update();
        }
    }

    if (CacheScene::getInstance().updateData() || CacheScene::getInstance().updateSizes() ||
        CacheScene::getInstance().updateShadows()) {
        for (int i = 0; i < scene->omniLights.size(); i++) {
            const auto& light = scene->omniLights[i];
            if (light->visible()) {
                if (light->shadow() && light->hasShadow()) {
                    light->shadow()->update(m_dataOmni->lights[i].position);
                }
            }
        }
    }
    //m_omniLights->modifyData(0, sizeof(glm::vec4), value_ptr(omniLength));
    //m_omniLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(LightType::LightOmni),m_dataOmni->lights.data());
}

void Prisma::LightHandler::updateCSM() {
    const auto& dirLights = GlobalData::getInstance().currentGlobalScene()->dirLights;

    if (dirLights.size() > 0 && dirLights[0]->shadow() && dirLights[0]->hasShadow() && m_updateCascade &&
        Engine::getInstance().engineSettings().pipeline != EngineSettings::Pipeline::RAYTRACING) {
        auto shadow = dirLights[0]->shadow();

        const auto& dirMatrix = dirLights[0]->finalMatrix();

        const auto& dirMult = normalize(dirMatrix * dirLights[0]->type().direction);

        shadow->update(dirMult);
    }
}


void Prisma::LightHandler::updateSizes() {
    auto& contextData = PrismaFunc::getInstance().contextData();
    contextData.immediateContext->UpdateBuffer(m_lightSizes, 0, sizeof(LightSizes), &m_sizes,
                                               Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

std::vector<Diligent::IDeviceObject*>& Prisma::LightHandler::omniData() {
    return m_omniData;
}

Diligent::IDeviceObject* Prisma::LightHandler::dirShadowData() {
    const auto& dirLights = GlobalData::getInstance().currentGlobalScene()->dirLights;

    if (dirLights.size() > 0 && dirLights[0]->shadow() && dirLights[0]->hasShadow()) {
        auto shadow = dirLights[0]->shadow();

        return shadow->shadowTexture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    }
    return GlobalData::getInstance().dummyTextureArray()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
}

void Prisma::LightHandler::addLightHandler(std::function<void()> update) {
    m_updates.push_back(update);
}

bool Prisma::LightHandler::updateCascade() {
    return m_updateCascade;
}

void Prisma::LightHandler::updateCascade(bool updateCascade) { m_updateCascade = updateCascade; }

void Prisma::LightHandler::update() {
    const auto& scene = GlobalData::getInstance().currentGlobalScene();
    updateCSM();

    if (m_init || CacheScene::getInstance().updateData() || CacheScene::getInstance().updateSizes() ||
        CacheScene::getInstance().updateLights() || CacheScene::getInstance().updateStatus() ||
        CacheScene::getInstance().updateSizeLights() || CacheScene::getInstance().updateShadows()) {
        if (scene->dirLights.size() < Define::MAX_DIR_LIGHTS && scene->omniLights.size() <
            Define::MAX_OMNI_LIGHTS && scene->areaLights.size() < Define::MAX_AREA_LIGHTS) {
            updateDirectional();
            updateOmni();
            updateArea();
            updateSizes();
        } else {
            Logger::getInstance().log(LogLevel::ERRORS, "Too many lights");
        }
    }

    if (m_init || CacheScene::getInstance().updateSizeLights()) {
        for (auto update : m_updates) {
            update();
        }
    }

    m_clusterCalculation->updateCamera();
    m_clusterCalculation->updateLights();
    CacheScene::getInstance().resetCaches();

    m_init = false;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::LightHandler::lightSizes() const {
    return m_lightSizes;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::LightHandler::dirLights() const {
    return m_dirLights;
}

std::shared_ptr<Prisma::LightHandler::SSBODataArea> Prisma::LightHandler::dataArea() const {
    return m_dataArea;
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::LightHandler::omniLights() {
    return m_omniLights;
}

Prisma::LightHandler::ClusterData Prisma::LightHandler::clusters() {
    return {m_clusterCalculation->clusters(), m_clusterCalculation->clusterData()};
}