#pragma once

#include "SceneObjects/Light.h"
#include "GlobalData/GlobalData.h"
#include "Helpers/PrismaMath.h"
#include "glm/gtx/matrix_decompose.hpp"
#include <functional>
#include "NodeViewer.h"

namespace Prisma
{
	class LightInfo
	{
	public:
		void showSelectedDir(std::shared_ptr<Light<LightType::LightDir>> lightData, const NodeViewer::NodeData& meshData);
		void showSelectedOmni(std::shared_ptr < Light<LightType::LightOmni>> lightData, const NodeViewer::NodeData& meshData);
		void showSelectedArea(std::shared_ptr < Light<LightType::LightArea>> lightData, const NodeViewer::NodeData& meshData);

	private:
		glm::vec3 directionToEulerAngles(const glm::vec3& direction);
	};
}
