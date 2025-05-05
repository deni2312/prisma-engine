#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../GlobalData/Defines.h"
#include "../GlobalData/InstanceData.h"
#include <vector>

#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent {
struct IBuffer;
}

namespace Prisma {
class AnimationHandler : public InstanceData<AnimationHandler> {
public:
    struct SSBOAnimation {
        glm::mat4 animations[Define::MAX_BONES];
    };

    void fill();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> animation();

    AnimationHandler();

private:
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_animation;
};
}