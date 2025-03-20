#include "../../include/Containers/SSBO.h"
#include "../../include/Helpers/GarbageCollector.h"
#include <iostream>
#include <map>

static std::map<unsigned int, bool> usedId;

Prisma::SSBO::SSBO(unsigned int ssbo)
{
	/*if (usedId.find(ssbo) != usedId.end())
	{
		std::cerr << "SSBO ID " << ssbo << " ALREADY USED" << std::endl;
	}
	else
	{
		usedId[ssbo] = true;
		glGenBuffers(1, &m_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssbo, m_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		GarbageCollector::getInstance().add({GarbageCollector::GarbageType::BUFFER, m_ssbo});
		m_id = ssbo;
	}*/
}

void Prisma::SSBO::resize(unsigned int size, unsigned int type)
{
	/*bind();
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, type);
	unbind();*/
}

void Prisma::SSBO::modifyData(unsigned int offset, unsigned int size, void* data)
{
	/*glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_id, m_ssbo);
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/
}

void Prisma::SSBO::bind()
{
	/*glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_id, m_ssbo);*/
}

void Prisma::SSBO::getData(unsigned int size, void* data)
{
	/*glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/
}

void Prisma::SSBO::unbind()
{
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
