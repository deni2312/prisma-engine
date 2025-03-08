#pragma once
#include <string>
#include "../GlobalData/InstanceData.h"


namespace Prisma {
	class WindowsHelper : public InstanceData<WindowsHelper> {
	public:
		struct WindowsData
		{
			void* hInstance;
			int nShowCmd;
		};

		WindowsHelper();
		std::string openFolder(const std::string& stringFilter) const;
		std::string relativePath(const std::string& path) const;
	};
}