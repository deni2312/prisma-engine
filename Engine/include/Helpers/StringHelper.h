#pragma once
#include <string>
#include <filesystem>


namespace Prisma
{
	namespace fs = std::filesystem;

	class StringHelper
	{
	public:
		static bool endsWith(const std::string& value, const std::string& ending)
		{
			if (ending.size() > value.size()) return false;
			return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
		};

		std::string toRelativePath(const std::string& absolutePath, const std::string& basePath)
		{
			fs::path absPath(absolutePath);
			fs::path base(basePath);

			// Convert to relative path
			fs::path relativePath = fs::relative(absPath, base);

			return relativePath.string(); // Return as string
		}
	};
}
