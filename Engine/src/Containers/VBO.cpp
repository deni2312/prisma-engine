#include "../../include/Containers/VBO.h"
#include "../../include/Helpers/GarbageCollector.h"

Prisma::VBO::VBO()
{
	glGenBuffers(1, &m_id);
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::BUFFER, m_id});
}

void Prisma::VBO::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void Prisma::VBO::writeData(unsigned int size, void* offset, unsigned int type)
{
	bind();
	glBufferData(GL_ARRAY_BUFFER, size, offset, type);
}

void Prisma::VBO::writeSubData(unsigned int size, unsigned int offset, void* data)
{
	bind();
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

unsigned int Prisma::VBO::id() const
{
	return m_id;
}
