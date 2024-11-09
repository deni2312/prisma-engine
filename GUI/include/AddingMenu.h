#pragma once
#include "ImGuiCamera.h"
#include <imgui.h>
#include "MeshCreator.h"

namespace Prisma
{
	class ImGuiAddingMenu
	{
	public:
		void addMenu(Prisma::ImGuiCamera& camera) const;

	private:
		const int m_subDivisions = 16;
	};
}
