#pragma once

#include "GL/glew.h"

namespace Prisma
{
	class VAO
	{
	public:
		VAO();

		void bind();

		void addAttribPointer(unsigned int id, unsigned int stride, unsigned int size, void* offset,
		                      unsigned int type = GL_FLOAT);

		static void resetVao();

		unsigned int id() const;

	private:
		unsigned int m_id;
	};
}
