#include "../../include/GlobalData/GlobalData.h"


namespace Prisma {
    std::shared_ptr<Prisma::Scene> currentGlobalScene = nullptr;
    glm::mat4 currentProjection = glm::mat4(1.0f);
    Prisma::Texture defaultBlack;
    Prisma::Texture defaultWhite;
    Prisma::Texture defaultNormal;
    std::shared_ptr<Prisma::FBO> fboTarget = nullptr;

}