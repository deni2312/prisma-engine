#include "../../include/Containers/EBO.h"
#include "../../include/Helpers/GarbageCollector.h"

Prisma::EBO::EBO() {
    glGenBuffers(1, &m_id);
    Prisma::GarbageCollector::getInstance().add({Prisma::GarbageCollector::GarbageType::BUFFER,m_id});
}

void Prisma::EBO::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void Prisma::EBO::writeData(unsigned int size, void *offset, unsigned int type) {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, offset, type);
}

void Prisma::EBO::writeSubData(unsigned int size, unsigned int offset, void* data)
{
    bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset,size,data);
}

unsigned int Prisma::EBO::id() const {
    return m_id;
}

