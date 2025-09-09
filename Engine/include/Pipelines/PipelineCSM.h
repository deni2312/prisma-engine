#pragma once

#include <vector>
#include "GenericShadow.h"
#include "glm/glm.hpp"
#include "../Helpers/Settings.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Handlers/CSMHandler.h"

namespace Diligent {
struct ITexture;
}

namespace Prisma {
class PipelineCSM : public GenericShadow {
public:
    PipelineCSM(unsigned int width, unsigned int height, bool post = false);
    float farPlane() override;
    void farPlane(float farPlane) override;
    void init() override;
    float nearPlane() override;
    void nearPlane(float nearPlane) override;

    void lightDir(glm::vec3 lightDir);

    glm::vec3 lightDir();

    void update(glm::vec3 lightPos) override;

    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);
    Diligent::RefCntAutoPtr<Diligent::ITexture> shadowTexture() override;

    glm::mat4 getLightSpaceMatrix(float nearPlane, float farPlane);

    void createLightSpaceMatrices();

    void bias(float bias);

    float bias();

private:
    unsigned int m_width;

    unsigned int m_height;

    unsigned int m_fbo;

    glm::vec3 m_lightDir = glm::vec3(0.0f);

    uint64_t m_id;

    float m_farPlane = 1000.0f;

    float m_nearPlane = 0.1f;

    float m_bias = 0.5f;

    Settings m_settings;

    bool m_init = false;

    unsigned int m_size = 5;

    CSMHandler::CSMShadow m_lightMatrices;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_depth;

};
}