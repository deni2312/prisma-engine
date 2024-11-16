#include "../../include/Handlers/LoadingHandler.h"
#include "../../include/SceneData/MeshIndirect.h"

void Prisma::LoadingHandler::load(std::string scene, Prisma::SceneLoader::SceneParameters sceneParameters)
{
	m_sceneParameters = sceneParameters;
	m_loader.loadSceneAsync(scene, sceneParameters);
	m_hasLoad = true;
}

void Prisma::LoadingHandler::update(std::shared_ptr<Camera> camera)
{
	auto hasFinish = m_loader.hasFinish();
	if (hasFinish && m_hasLoad)
	{
		Prisma::GlobalData::getInstance().currentGlobalScene(hasFinish);
		Prisma::GlobalData::getInstance().currentGlobalScene()->camera = camera;
		Prisma::MeshIndirect::getInstance().init();
		m_sceneParameters.onLoad(hasFinish);
		m_hasLoad = false;
	}
}
