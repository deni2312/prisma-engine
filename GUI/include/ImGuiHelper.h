#pragma once
#include "GlobalData/InstanceData.h"
#include <functional>


namespace Prisma {
	class ImGuiHelper : public InstanceData<ImGuiHelper> {
	public:
		void clipVertical(int size, std::function<void(int)> data);
	};
}