#pragma once
#include "GlobalData/InstanceData.h"
#include "GlobalData/Platform.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <functional>


namespace Diligent {
struct IShaderBindingTable;
struct IBuffer;
struct ITopLevelAS;
struct TLASBuildInstanceData;
}

namespace Prisma {
class UpdateTLAS : public InstanceData<UpdateTLAS> {
public:
    UpdateTLAS();
    void update();

    void updateSizeTLAS();

    Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> TLAS();

    Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> SBT();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexData();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> primitiveData();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexLocation();

    void addUpdates(std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>,
                                       Diligent::RefCntAutoPtr<Diligent::IBuffer>,
                                       Diligent::RefCntAutoPtr<Diligent::IBuffer>)> update);

private:
    Diligent::RefCntAutoPtr<Diligent::ITopLevelAS> m_pTLAS;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_InstanceBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_ScratchBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vertexData;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_primitiveData;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vertexLocation;
    Diligent::RefCntAutoPtr<Diligent::IShaderBindingTable> m_pSBT;

    std::vector<std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>,
                                   Diligent::RefCntAutoPtr<Diligent::IBuffer>,
                                   Diligent::RefCntAutoPtr<Diligent::IBuffer>)>> m_updates;

    void updateTLAS(bool update);

    void resizeTLAS();

    struct VertexBlas {
        glm::vec4 norm;
        glm::vec4 uv;
        glm::vec4 tangent;
        glm::vec4 bitangent;
    };

    struct LocationBlas {
        int location;
        int size;
        int locationPrimitive;
        float padding;
    };
};
}