#include "../../include/Helpers/GarbageCollector.h"
#include "gl/glew.h"

Prisma::GarbageCollector::GarbageCollector()
{
}

void Prisma::GarbageCollector::add(std::pair<GarbageType, unsigned int> garbage)
{
	m_garbage.push_back(garbage);
}

void Prisma::GarbageCollector::addTexture(std::pair<unsigned int, uint64_t> texture)
{
	m_garbageTexture.push_back(texture);
}

void Prisma::GarbageCollector::clear()
{
	for (auto garbage : m_garbage)
	{
		switch (garbage.first)
		{
		case VAO:
			glDeleteVertexArrays(1, &garbage.second);
			break;
		case BUFFER:
			glDeleteBuffers(1, &garbage.second);
			break;
		case TEXTURE:
			glDeleteTextures(1, &garbage.second);
			break;
		case FBO:
			glDeleteFramebuffers(1, &garbage.second);
			break;
		case RBO:
			glDeleteRenderbuffers(1, &garbage.second);
			break;
		}
	}

	for (auto garbage : m_garbageTexture)
	{
		glMakeImageHandleNonResidentARB(garbage.second);
		glDeleteTextures(1, &garbage.first);
	}
}
