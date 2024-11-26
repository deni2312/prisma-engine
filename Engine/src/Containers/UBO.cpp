#include "../../include/Containers/Ubo.h"
#include "../../include/Helpers/GarbageCollector.h"
#include <iostream>

static std::vector<unsigned int> usedId;

Prisma::Ubo::Ubo(unsigned int size, int ubo)
{
	auto find = std::find(usedId.begin(), usedId.end(), ubo);
	if (find != usedId.end())
	{
		std::cerr << "UBO ID " << ubo << " ALREADY USED" << std::endl;
	}
	else
	{
		usedId.push_back(ubo);
		glGenBuffers(1, &m_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		glBindBufferRange(GL_UNIFORM_BUFFER, ubo, m_ubo, 0, size);
		GarbageCollector::getInstance().add({GarbageCollector::GarbageType::BUFFER, m_ubo});
	}
}

void Prisma::Ubo::modifyData(unsigned int offset, unsigned int size, void* data)
{
	glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
