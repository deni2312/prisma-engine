#pragma once
#include <string>
#include <filesystem>
#include "../GlobalData/InstanceData.h"


namespace Prisma
{
	namespace fs = std::filesystem;

	class StringHelper : public InstanceData<StringHelper>
	{
	public:
		bool endsWith(const std::string& value, const std::string& ending);;

		std::string toRelativePath(const std::string& absolutePath, const std::string& basePath);
	};
}
