#pragma once
#include <memory>
#include "glm/glm.hpp"
#include "../Helpers/Settings.h"

#include "../GlobalData/InstanceData.h"
#include "Common/interface/RefCntAutoPtr.hpp"

namespace Diligent {
struct IBuffer;
}

namespace Prisma {
class MeshHandler : public InstanceData<MeshHandler> {
public:
    struct ViewProjectionData {
        glm::mat4 view;
        glm::mat4 projection;
        glm::vec4 viewPos;
    };

    struct alignas(16) ClusterData {
        glm::uvec4 gridSize;
        glm::uvec4 screenDimensions;
        float zNear;
        float zFar;
        float padding[2];
    };

    std::shared_ptr<ViewProjectionData> data() const;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> viewProjection() const;
    void updateCamera();
    void updateCluster();
    MeshHandler();

private:
    ClusterData m_clusterData;
    Settings m_settings;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_viewProjection;
};
}