#pragma once
#include <memory>

#include "Components/RenderComponent.h"


namespace Prisma {
class TreeRendererComponent : public RenderComponent {
   public:
    TreeRendererComponent();

    void ui() override;

    void start() override;

   private:

};
}  // namespace Prisma