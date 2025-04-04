#include "../../include/Handlers/LoadingHandler.h"
#include "../../include/SceneData/MeshIndirect.h"
#include "../../include/Helpers/StringHelper.h"

void Prisma::LoadingHandler::load(std::string scene, Prisma::SceneLoader::SceneParameters sceneParameters)
{
	if (Prisma::StringHelper::getInstance().endsWith(scene, "prisma"))
	{
		m_sceneParameters = sceneParameters;
		m_loader.loadSceneAsync(scene, sceneParameters);
		m_hasLoad = true;
	}
}

void Prisma::LoadingHandler::update(std::shared_ptr<Camera> camera,
                                    std::function<void(std::pair<std::string, int>)> loading)
{
	auto hasFinish = m_loader.hasFinish();

	if (!hasFinish && m_hasLoad)
	{
		m_loader.exporter().mutexData().lock();
		if (loading) {
			loading(m_loader.exporter().status());
		}
		m_loader.exporter().mutexData().unlock();
	}

	if (hasFinish && m_hasLoad)
	{
		if (m_sceneParameters.append)
		{
			Prisma::GlobalData::getInstance().currentGlobalScene()->root->addChild(hasFinish->root);
		}
		else
		{
			Prisma::GlobalData::getInstance().currentGlobalScene(hasFinish);
		}
		Prisma::MeshIndirect::getInstance().init();

		Prisma::GlobalData::getInstance().currentGlobalScene()->camera = camera;
		if (m_sceneParameters.onLoad)
		{
			m_sceneParameters.onLoad(hasFinish);
		}
		Prisma::CacheScene::getInstance().updateSizes(true);
		m_hasLoad = false;
	}
}
