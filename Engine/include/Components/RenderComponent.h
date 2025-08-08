#pragma once
#include "Component.h"

namespace Prisma {

class RenderComponent : public Component {
public:
    virtual void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {};

    virtual void updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {};
};
}