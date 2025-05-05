#pragma once
#include <memory>
#include "../GlobalData/Defines.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent {
struct IShaderResourceBinding;
struct IPipelineState;
struct IBuffer;
}

namespace Prisma {
class ClusterCalculation {
public:
    ClusterCalculation(Diligent::RefCntAutoPtr<Diligent::IBuffer> omniLights,
                       Diligent::RefCntAutoPtr<Diligent::IBuffer> lightSizes);
    void updateCamera();
    void updateLights();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> clusters();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> clusterData();

private:
    struct alignas(16) Cluster {
        glm::vec4 minPoint;
        glm::vec4 maxPoint;
        unsigned int count;
        unsigned int lightIndices[Define::MAX_CLUSTER_SIZE];
    };

    void createCamera();
    void createLight();
    static constexpr unsigned int m_gridSizeX = 12;
    static constexpr unsigned int m_gridSizeY = 12;
    static constexpr unsigned int m_gridSizeZ = 24;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_omniLights;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_lightSizes;

    struct ClusterData {
        glm::vec4 nearFar;
        glm::mat4 inverseProjection;
        glm::ivec4 gridSize;
        glm::ivec4 screenDimensions;
    };

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_clusterData;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_cluster;
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_psoLight;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srbLight;
    ClusterData m_data;
};
}