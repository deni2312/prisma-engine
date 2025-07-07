#pragma once

#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include <Common/interface/RefCntAutoPtr.hpp>
#include "Helpers/Blit.h"
#include "Component.h"
#include "Postprocess/CloudPostprocess.h"

namespace Prisma {
class CloudComponent : public Component {
   public:
    CloudComponent();

    void ui() override;

    void update() override;

    void start() override;

    void destroy() override;

   private:
    std::shared_ptr<Prisma::CloudPostprocess> m_clouds;
};
}