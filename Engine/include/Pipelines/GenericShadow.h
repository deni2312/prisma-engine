#pragma once
#include "glm/glm.hpp"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent {
struct ITexture;
}


namespace Prisma {
class GenericShadow {
public:
    virtual Diligent::RefCntAutoPtr<Diligent::ITexture> shadowTexture() { return Diligent::RefCntAutoPtr<Diligent::ITexture>(); };

    virtual float farPlane() = 0;
    virtual void farPlane(float farPlane) = 0;

    virtual float nearPlane() = 0;
    virtual void nearPlane(float nearPlane) = 0;

    virtual void init() = 0;

    virtual void update(glm::vec3 lightPos) =0;

    virtual ~GenericShadow() {
    } // Virtual destructor
};
}