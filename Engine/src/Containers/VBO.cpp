#include "../../include/Containers/VBO.h"
#include "../../include/Helpers/GarbageCollector.h"

Prisma::VBO::VBO() {
    glGenBuffers(1, &m_id);
    Prisma::GarbageCollector::getInstance().add({Prisma::GarbageCollector::GarbageType::BUFFER,m_id});
}

void Prisma::VBO::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void Prisma::VBO::writeData(unsigned int size, void *offset, unsigned int type) {
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, offset, type);
}

unsigned int Prisma::VBO::id() const {
    return m_id;
}
