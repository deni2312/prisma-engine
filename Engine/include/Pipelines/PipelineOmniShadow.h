#pragma once

#include <vector>
#include "GenericShadow.h"
#include "GlobalData/Platform.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent {
struct ITexture;
struct IBuffer;
struct ITextureView;
struct IShaderResourceBinding;
}

namespace Prisma {
class PipelineOmniShadow : public GenericShadow {
public:
    PipelineOmniShadow(unsigned int width, unsigned int height, bool post = false);
    float farPlane() override;
    void farPlane(float farPlane) override;
    void init() override;
    float nearPlane() override;
    void nearPlane(float nearPlane) override;

    void update(glm::vec3 lightPos) override;

    Diligent::RefCntAutoPtr<Diligent::ITexture> shadowTexture() override;

private:
    unsigned int m_width;
    unsigned int m_height;
    float m_nearPlane = 0.1f;

    std::vector<unsigned int> m_shadowPosition;

    std::vector<unsigned int> m_shadowPositionAnimation;

    uint64_t m_id;
    float m_farPlane = 100.0f;

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_depth;

    bool m_init = false;
};
}