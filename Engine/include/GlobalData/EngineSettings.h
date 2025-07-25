#pragma once

namespace Prisma::EngineSettings {
enum class Pipeline {
    FORWARD,
    DEFERRED_FORWARD,
    RAYTRACING,
    SOFTWARE_RAYTRACING
};

struct Settings {
    Pipeline pipeline;
    bool ssr;
    bool ssao;
};
}