#pragma once
#include <string>
#include <Windows.h>
#include "../GlobalData/InstanceData.h"


namespace Prisma {
	class WindowsHelper : public InstanceData<WindowsHelper> {
	public:
		WindowsHelper();
		std::string openFolder(const std::string& stringFilter);
	};
}