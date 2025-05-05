#pragma once
#include "Component.h"

namespace Prisma {
class DecalComponent : public Component {
public:
    DecalComponent();

    void start() override;
    //std::shared_ptr<Prisma::Shader> m_shader;
};
}