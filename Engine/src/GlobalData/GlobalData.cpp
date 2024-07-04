#include "../../include/GlobalData/GlobalData.h"


namespace Prisma {
    std::shared_ptr<Prisma::Scene> currentGlobalScene = nullptr;
    glm::mat4 currentProjection = glm::mat4(1.0f);
    bool updateLights = false;
    bool updateSizes = true;
    bool updateData = false;
    bool skipUpdate = false;
    bool updateTextures = false;
    Prisma::Texture defaultBlack;
    Prisma::Texture defaultNormal;
    std::shared_ptr<Prisma::FBO> fboTarget = nullptr;

    void resetCaches() {
        if (!skipUpdate) {
            updateLights = false;
            updateSizes = false;
            updateData = false;
            updateTextures = false;
        }
        else {
            skipUpdate = false;
        }
    }

}