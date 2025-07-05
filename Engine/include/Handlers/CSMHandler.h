#pragma once
#include "GlobalData/Platform.h"

#include "GlobalData/InstanceData.h"

#include <glm/glm.hpp>
#include <vector>
#include <Common/interface/RefCntAutoPtr.hpp>
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"

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

    void createShadow(unsigned int width,unsigned int height);

    struct CSMShadow {
        glm::mat4 shadows[16];
        glm::vec4 cascadePlanes[16];
        float sizeCSM;
        float farPlaneCSM;
        glm::vec2 resolutionCSM;
    };

    struct CSMData {
        CSMShadow shadows;
    };

    unsigned int m_width;
    unsigned int m_height;

    unsigned int m_size = 5;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> shadowBuffer();

    Diligent::RefCntAutoPtr<Diligent::ITexture> shadowTexture();

    void render(const CSMData& data);

    Diligent::RefCntAutoPtr<Diligent::ITexture> m_depth;
};
}