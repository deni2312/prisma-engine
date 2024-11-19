#include "../../include/Handlers/LoadingHandler.h"
#include "../../include/SceneData/MeshIndirect.h"

void Prisma::LoadingHandler::load(std::string scene, Prisma::SceneLoader::SceneParameters sceneParameters)
{
	m_sceneParameters = sceneParameters;
	m_loader.loadSceneAsync(scene, sceneParameters);
	m_hasLoad = true;
}

void Prisma::LoadingHandler::update(std::shared_ptr<Camera> camera,
                                    std::function<void(std::pair<std::string, int>)> loading)
{
	auto hasFinish = m_loader.hasFinish();

	if (!hasFinish && m_hasLoad)
	{
		m_loader.exporter().mutexData().lock();
		loading(m_loader.exporter().status());
		m_loader.exporter().mutexData().unlock();
	}

	if (hasFinish && m_hasLoad)
	{
		Prisma::GlobalData::getInstance().currentGlobalScene(hasFinish);
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera = camera;
		Prisma::MeshIndirect::getInstance().init();
		if (m_sceneParameters.onLoad)
		{
			m_sceneParameters.onLoad(hasFinish);
		}
		m_hasLoad = false;
	}
}
