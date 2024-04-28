#include "../../include/Containers/VAO.h"
#include "../../include/Helpers/GarbageCollector.h"

Prisma::VAO::VAO() {
    glGenVertexArrays(1, &m_id);
    Prisma::GarbageCollector::getInstance().add({Prisma::GarbageCollector::GarbageType::VAO,m_id});
}

void Prisma::VAO::bind() {
    glBindVertexArray(m_id);
}

void Prisma::VAO::addAttribPointer(unsigned int id, unsigned int stride, unsigned int size, void *offset, unsigned int type) {
    bind();
    glEnableVertexAttribArray(id);
    glVertexAttribPointer(id, stride, type, GL_FALSE, size, offset);
}

void Prisma::VAO::resetVao() {
    glBindVertexArray(0);
}

unsigned int Prisma::VAO::id() const {
    return m_id;
}
