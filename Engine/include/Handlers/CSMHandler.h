#pragma once
#include "GlobalData/Platform.h"

#include "GlobalData/InstanceData.h"

#include <glm/glm.hpp>
#include <vector>
#include <Common/interface/RefCntAutoPtr.hpp>

namespace Diligent {
struct IPipelineState;
struct ITexture;
struct IBuffer;
struct ITextureView;
struct IShaderResourceBinding;
}

namespace Prisma {
class CSMHandler : public InstanceData<CSMHandler> {
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_shadowBuffer;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoAnimation;

    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbAnimation;

    void create();

    void createAnimation();

public:
    CSMHandler();

    struct CSMShadow {
        glm::mat4 shadows[16];
        glm::vec4 cascadePlanes[16];
        float sizeCSM;
        float farPlaneCSM;
        glm::vec2 paddingCSM;
    };

    struct CSMData {
        Diligent::RefCntAutoPtr<Diligent::ITexture> depth;
        CSMShadow shadows;
    };

    Diligent::RefCntAutoPtr<Diligent::IBuffer> shadowBuffer();

    void render(const CSMData& data);
};
}