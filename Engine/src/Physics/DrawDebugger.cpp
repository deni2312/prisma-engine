
#include "../../include/Physics/DrawDebugger.h"
#include "../../include/Helpers/PrismaMath.h"

void Prisma::DrawDebugger::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    glm::vec3 from_object=Prisma::JfromVec3(inFrom);
    glm::vec3 to_object = Prisma::JfromVec3(inTo);
    glm::vec3 color_object;
    color_object.x = inColor.r;
    color_object.y = inColor.g;
    color_object.z = inColor.b;
    line.setColor(color_object);
    line.draw(from_object, to_object);
}

void Prisma::DrawDebugger::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor, float inHeight)
{
}
