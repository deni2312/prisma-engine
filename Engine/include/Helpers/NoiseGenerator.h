#pragma once

#include <Common/interface/RefCntAutoPtr.hpp>

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "../GlobalData/InstanceData.h"
#include "glm/glm.hpp"

namespace Prisma {
class NoiseGenerator : public InstanceData<NoiseGenerator> {
   public:
    Diligent::RefCntAutoPtr<Diligent::ITexture> generate(const std::string& vertex, const std::string& fragment, glm::vec2 resolution, const std::string& name);

};
}  // namespace Prisma