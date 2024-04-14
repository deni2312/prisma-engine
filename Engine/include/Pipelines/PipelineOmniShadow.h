#pragma once

#include <vector>
#include "../Helpers/Shader.h"
#include "GenericShadow.h"

namespace Prisma {

    class PipelineOmniShadow : public GenericShadow{
    public:
        PipelineOmniShadow(unsigned int width, unsigned int height);
        uint64_t id();
        float farPlane() override;
        void farPlane(float farPlane) override;

        float nearPlane() override;
        void nearPlane(float nearPlane) override;

        void update(glm::vec3 lightPos) override;
    private:
        unsigned int m_width;
        unsigned int m_height;
        float m_nearPlane=1;

        unsigned int m_fbo;
        unsigned int m_farPlanePos;
        unsigned int m_lightPos;
        std::vector<unsigned int> m_shadowPosition;
        uint64_t m_id;
        float m_farPlane=100;

        glm::mat4 m_shadowProj;
        std::vector<glm::mat4> m_shadowTransforms;
    };
}