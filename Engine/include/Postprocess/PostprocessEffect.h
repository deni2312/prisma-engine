#pragma once
#include <iostream>

namespace Prisma
{
	class PostprocessEffect
	{
	public:
		/*virtual void render(std::shared_ptr<FBO> texture, std::shared_ptr<FBO> raw)
		{
		};*/

	private:
		bool m_raw = false;
	};
}
