#pragma once

#include <vector>
#include "../Helpers/Shader.h"
#include "GenericShadow.h"
#include "glm/glm.hpp"
#include "../Containers/Ubo.h"
#include "../Helpers/Settings.h"
#include "../Containers/SSBO.h"

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

        std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projview);

        std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);

        glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane);

        std::vector<glm::mat4> getLightSpaceMatrices();

        std::vector<float>& cascadeLevels();

        void bias(float bias);

        float bias();

    private:

        unsigned int m_width;

        unsigned int m_height;

        unsigned int m_fbo;

        glm::vec3 m_lightDir=glm::vec3(0.0f);

        uint64_t m_id;

        float m_farPlane = 200.0f;

        float m_nearPlane = 0.1f;

        float m_bias = 0.5f;

        std::vector<float> m_shadowCascadeLevels;

        Prisma::Settings m_settings;

        std::shared_ptr<Prisma::SSBO> m_ssbo = nullptr;

        bool m_init = false;

        glm::vec3 m_boundingBoxMin;

        glm::vec3 m_boundingBoxMax;

        std::vector<glm::mat4> m_lightMatrices;

        uint64_t m_numCSM;

        std::shared_ptr<Prisma::Shader> m_shader = nullptr;

        std::shared_ptr<Prisma::Shader> m_shaderAnimation = nullptr;

    };
}