
#include "../../include/Physics/DrawDebugger.h"

void Prisma::DrawDebugger::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
    glm::vec3 from_object;
    from_object.x=from.getX();
    from_object.y=from.getY();
    from_object.z=from.getZ();
    glm::vec3 to_object;
    to_object.x=to.getX();
    to_object.y=to.getY();
    to_object.z=to.getZ();
    glm::vec3 color_object;
    color_object.x=color.getX();
    color_object.y=color.getY();
    color_object.z=color.getZ();
    line.setColor(color_object);
    line.draw(from_object,to_object);
}

void
Prisma::DrawDebugger::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime,
                               const btVector3 &color) {

}

void Prisma::DrawDebugger::reportErrorWarning(const char *warningString) {
    std::cout<<*warningString;
}

void Prisma::DrawDebugger::draw3dText(const btVector3 &location, const char *textString) {

}

void Prisma::DrawDebugger::setDebugMode(int debugMode) {
    debug=debugMode;
}

int Prisma::DrawDebugger::getDebugMode() const {
    return debug;
}
