#pragma once
#include "../GlobalData/InstanceData.h"
#include <string>
#include "../SceneData/SceneLoader.h"

namespace Prisma
{
	class LoadingHandler : public InstanceData<LoadingHandler>
	{
	public:
		void load(std::string scene, Prisma::SceneLoader::SceneParameters sceneParameters);
		void update(std::shared_ptr<Camera> camera, std::function<void(std::pair<std::string, int>)> loading);

	private:
		Prisma::SceneLoader::SceneParameters m_sceneParameters;
		Prisma::SceneLoader m_loader;
		bool m_hasLoad = false;
	};
}
