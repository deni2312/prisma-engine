#pragma once

#include "GL/glew.h"

namespace Prisma
{
	class VBO
	{
	public:
		VBO();

		void bind();

		void writeData(unsigned int size, void* offset, unsigned int type = GL_STATIC_DRAW);
		void writeSubData(unsigned int size, unsigned int offset, void* data);

		unsigned int id() const;

	private:
		unsigned int m_id;
	};
}
