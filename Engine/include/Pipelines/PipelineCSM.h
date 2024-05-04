#pragma once

#include <vector>
#include "../Helpers/Shader.h"
#include "GenericShadow.h"
#include "glm/glm.hpp"
#include "../Containers/Ubo.h"

namespace Prisma {

    class PipelineCSM : public GenericShadow {
    public:
        PipelineCSM(unsigned int width, unsigned int height);
        uint64_t id();
        float farPlane() override;
        void farPlane(float farPlane) override;

        float nearPlane() override;
        void nearPlane(float nearPlane) override;

        void lightDir(glm::vec3 lightDir);

        glm::vec3 lightDir();

        void update(glm::vec3 lightPos) override;

        glm::mat4 lightMatrix();

        void projectionLength(glm::vec4 projectionLength);

        glm::vec4 projectionLength();

    private:
        unsigned int m_width;
        unsigned int m_height;

        unsigned int m_fbo;

        unsigned int m_posLightmatrix;
        unsigned int m_posLightmatrixAnimation;

        glm::vec3 m_lightDir=glm::vec3(0.0f);

        glm::mat4 m_lightSpaceMatrix;

        uint64_t m_id;
        float m_farPlane = 100;
        float m_nearPlane = 0.1f;
        glm::vec4 m_projectionLength;

        std::shared_ptr<Prisma::Ubo> m_ubo;

    };
}