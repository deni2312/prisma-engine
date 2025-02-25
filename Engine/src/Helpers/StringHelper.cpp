#include "../../include/Helpers/StringHelper.h"

bool Prisma::StringHelper::endsWith(const std::string& value, const std::string& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string Prisma::StringHelper::toRelativePath(const std::string& absolutePath, const std::string& basePath)
{
	fs::path absPath(absolutePath);
	fs::path base(basePath);

	// Convert to relative path
	fs::path relativePath = fs::relative(absPath, base);

	return relativePath.string(); // Return as string
}

std::string Prisma::StringHelper::toLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
	               [](unsigned char c) { return std::tolower(c); }
	);
	return s;
}
