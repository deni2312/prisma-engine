#include "../../include/Helpers/GarbageCollector.h"
#include "gl/glew.h"

std::shared_ptr<Prisma::GarbageCollector> Prisma::GarbageCollector::instance = nullptr;

Prisma::GarbageCollector &Prisma::GarbageCollector::getInstance() {
    if (!instance) {
        instance = std::make_shared<GarbageCollector>();
    }
    return *instance;
}

Prisma::GarbageCollector::GarbageCollector() {

}

void Prisma::GarbageCollector::add(std::pair<GarbageType, unsigned int> garbage) {
    m_garbage.push_back(garbage);
}

void Prisma::GarbageCollector::clear() {
    for(auto garbage : m_garbage){
        switch(garbage.first){

            case VAO:
                glDeleteVertexArrays(1,&garbage.second);
                break;
            case BUFFER:
                glDeleteBuffers(1,&garbage.second);
                break;
            case TEXTURE:
                glDeleteTextures(1,&garbage.second);
                break;
            case FBO:
                glDeleteFramebuffers(1,&garbage.second);
                break;
            case RBO:
                glDeleteRenderbuffers(1,&garbage.second);
                break;
        }
    }
}
