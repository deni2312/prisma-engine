#include "Handlers/LoadingHandler.h"
#include "SceneData/MeshIndirect.h"
#include "Helpers/StringHelper.h"

void Prisma::LoadingHandler::load(std::string scene, SceneLoader::SceneParameters sceneParameters) {
        if (StringHelper::getInstance().endsWith(scene, "prisma")) {
                m_sceneParameters = sceneParameters;
                m_loader.loadSceneAsync(scene, sceneParameters);
                m_hasLoad = true;
        }
}

void Prisma::LoadingHandler::update(std::shared_ptr<Camera> camera,
                                    std::function<void(std::pair<std::string, int>)> loading) {
        auto hasFinish = m_loader.hasFinish();

        if (!hasFinish && m_hasLoad) {
                m_loader.exporter().mutexData().lock();
                if (loading) {
                        loading(m_loader.exporter().status());
                }
                m_loader.exporter().mutexData().unlock();
        }

        if (hasFinish && m_hasLoad) {
                if (m_sceneParameters.append) {
                        GlobalData::getInstance().currentGlobalScene()->root->addChild(hasFinish->root);
                } else {
                        GlobalData::getInstance().currentGlobalScene(hasFinish);
                }

                GlobalData::getInstance().currentGlobalScene()->camera = camera;
                if (m_sceneParameters.onLoad) {
                        m_sceneParameters.onLoad(hasFinish);
                }
                MeshIndirect::getInstance().init();
                m_hasLoad = false;
        }
}