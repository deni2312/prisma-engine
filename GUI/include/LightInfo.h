#pragma once

#include "../../Engine/include/SceneObjects/Light.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "imgui.h"
#include "../../Engine/include/Helpers/PrismaMath.h"
#include "glm/gtx/matrix_decompose.hpp"
#include <functional>
#include "ImGuizmo.h"
#include "MeshInfo.h"

namespace Prisma
{
	class LightInfo
	{
	public:
		void showSelectedDir(Light<LightType::LightDir>* lightData, const NodeViewer::NodeData& meshData);
		void showSelectedOmni(Light<LightType::LightOmni>* lightData, const NodeViewer::NodeData& meshData);
		void showSelectedArea(Light<LightType::LightArea>* lightData, const NodeViewer::NodeData& meshData);

	private:
		glm::vec3 directionToEulerAngles(const glm::vec3& direction);
	};
}
