#pragma once


#include <memory>

namespace Prisma
{
	class PipelineFullScreen
	{
	public:
		PipelineFullScreen();
		void render(uint64_t texture);

	private:
		//std::shared_ptr<Shader> m_shader;
		unsigned int m_bindlessPos;
	};
}
