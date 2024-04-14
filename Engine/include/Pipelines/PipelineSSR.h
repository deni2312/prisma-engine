#pragma once

#include <memory>
#include "../Helpers/Shader.h"

namespace Prisma {
    class PipelineSSR {
    public:
        PipelineSSR();

        void update(uint64_t albedo,uint64_t metalness,uint64_t normal);
    private:
        std::shared_ptr<Shader> m_shader;
        unsigned int m_albedoPos;
        unsigned int m_normalPos;
        unsigned int m_metalnessPos;
        unsigned int m_samplingPos;
    };
}
