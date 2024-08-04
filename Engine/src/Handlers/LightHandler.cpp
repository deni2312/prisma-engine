#include "../../include/Handlers/LightHandler.h"
#include "../../include/GlobalData/GlobalData.h"
#include "glm/gtx/string_cast.hpp"
#include <iostream>
#include "../../include/GlobalData/CacheScene.h"

std::shared_ptr<Prisma::LightHandler> Prisma::LightHandler::instance = nullptr;

Prisma::LightHandler::LightHandler()
{
    m_dirLights = std::make_shared<Prisma::SSBO>(2);
    m_dirLights->resize(MAX_DIR_LIGHTS * sizeof(Prisma::LightType::LightDir)+sizeof(glm::vec4));

    m_omniLights = std::make_shared<Prisma::SSBO>(3);
    m_omniLights->resize(MAX_OMNI_LIGHTS * sizeof(Prisma::LightType::LightOmni) + sizeof(glm::vec4));

    m_dirCSM = std::make_shared<Prisma::SSBO>(9);
    m_dirCSM->resize(16 * sizeof(float) + sizeof(glm::vec4));

    glm::vec3 size = ClusterCalculation::grids();

    m_clusterCalculation = std::make_shared<ClusterCalculation>(size.x * size.y * size.z);

    m_init=true;
}

void Prisma::LightHandler::updateDirectional()
{
    const auto& scene = currentGlobalScene;

    if (scene->dirLights.size()>0) {

        auto shadow = currentGlobalScene->dirLights[0]->shadow();

        if (shadow && currentGlobalScene->dirLights[0]->hasShadow()) {

            auto& levels = std::dynamic_pointer_cast<PipelineCSM>(shadow)->cascadeLevels();
            glm::vec4 length;
            length.x = levels.size();
            length.y = shadow->farPlane();

            m_dirCSM->modifyData(0, 16 * sizeof(float), levels.data());
            m_dirCSM->modifyData(16 * sizeof(float), sizeof(glm::vec4), glm::value_ptr(length));
        }
    }

    m_dataDirectional = std::make_shared<Prisma::LightHandler::SSBODataDirectional>();
    for (int i = 0; i < scene->dirLights.size(); i++) {
        const auto& light = scene->dirLights[i];
        m_dataDirectional->lights.push_back(scene->dirLights[i]->type());
        const auto& dirMatrix = scene->dirLights[i]->finalMatrix();
        auto shadow = std::dynamic_pointer_cast<PipelineCSM>(light->shadow());
        if (shadow) {
            shadow->lightDir(m_dataDirectional->lights[i].direction);
            shadow->update(dirMatrix * m_dataDirectional->lights[i].direction);
        }
        m_dataDirectional->lights[i].direction = dirMatrix * m_dataDirectional->lights[i].direction;
        m_dataDirectional->lights[i].padding.x = scene->dirLights[i]->hasShadow() ? 2.0f : 0.0f;
    }
    glm::vec4 dirLength;
    dirLength.r = scene->dirLights.size();
    m_dirLights->modifyData(0, sizeof(glm::vec4),
        glm::value_ptr(dirLength));
    m_dirLights->modifyData(sizeof(glm::vec4), scene->dirLights.size() * sizeof(Prisma::LightType::LightDir),
        m_dataDirectional->lights.data());
}

void Prisma::LightHandler::updateOmni()
{
    const auto& scene = currentGlobalScene;

    m_dataOmni = std::make_shared<Prisma::LightHandler::SSBODataOmni>();
    int numVisible = 0;
    for (int i = 0; i < scene->omniLights.size(); i++) {
        const auto& light = scene->omniLights[i];
        if (light->visible()) {
            m_dataOmni->lights.push_back(light->type());
            glm::mat4 omniMatrix;
            if (light->parent()) {
                omniMatrix = light->parent()->finalMatrix();
            }
            else {
                omniMatrix = light->matrix();
            }
            m_dataOmni->lights[i].position = omniMatrix * m_dataOmni->lights[i].position;
            if (light->shadow()) {
                light->shadow()->update(m_dataOmni->lights[i].position);
                m_dataOmni->lights[i].farPlane.x = light->shadow()->farPlane();
            }
            m_dataOmni->lights[i].padding = scene->omniLights[i]->hasShadow() ? 2.0f : 0.0f;
            numVisible++;
        }
    }

    glm::vec4 omniLength;
    omniLength.r = numVisible;
    m_omniLights->modifyData(0, sizeof(glm::vec4),
        glm::value_ptr(omniLength));

    m_omniLights->modifyData(sizeof(glm::vec4), numVisible * sizeof(Prisma::LightType::LightOmni),
        m_dataOmni->lights.data());
}

void Prisma::LightHandler::updateCSM()
{
    const auto& scene = currentGlobalScene;

    if (scene->dirLights.size() > 0) {

        auto shadow = currentGlobalScene->dirLights[0]->shadow();

        const auto& dirMatrix = scene->dirLights[0]->finalMatrix();

        shadow->update(dirMatrix * currentGlobalScene->dirLights[0]->type().direction);
    }
}

Prisma::LightHandler& Prisma::LightHandler::getInstance()
{
    if (!instance) {
        instance = std::make_shared<LightHandler>();
    }
    return *instance;
}

void Prisma::LightHandler::update()
{
    const auto& scene = currentGlobalScene;

    updateCSM();

    if(m_init || Prisma::CacheScene::getInstance().updateData() || Prisma::CacheScene::getInstance().updateSizes() || Prisma::CacheScene::getInstance().updateLights()) {
        if (scene->dirLights.size() < MAX_DIR_LIGHTS && scene->omniLights.size() < MAX_OMNI_LIGHTS) {
            updateDirectional();
            updateOmni();
        } else {
            std::cerr << "Too many lights" << std::endl;
        }
    }

    m_clusterCalculation->updateCamera();
    m_clusterCalculation->updateLights();
    Prisma::CacheScene::getInstance().resetCaches();
    m_init=false;
}

void Prisma::LightHandler::bind()
{
    m_dirLights->bind();
}

std::shared_ptr<Prisma::LightHandler::SSBODataDirectional> Prisma::LightHandler::dataDirectional() const
{
    return m_dataDirectional;
}

std::shared_ptr<Prisma::SSBO> Prisma::LightHandler::ssboDirectional() const
{
    return m_dirLights;
}

std::shared_ptr<Prisma::LightHandler::SSBODataOmni> Prisma::LightHandler::dataOmni() const
{
    return m_dataOmni;
}

std::shared_ptr<Prisma::SSBO> Prisma::LightHandler::ssboOmni() const
{
    return m_omniLights;
}
