#pragma once
#include <string>

namespace Prisma
{
	class StringHelper
	{
	public:
		static bool endsWith(const std::string& value, const std::string& ending)
		{
			if (ending.size() > value.size()) return false;
			return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
		};
	};
}
