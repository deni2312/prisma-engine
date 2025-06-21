#include "Handlers/AnimationHandler.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/Defines.h"
#include "Helpers/TimeCounter.h"
#include "SceneData/MeshIndirect.h"


void Prisma::AnimationHandler::fill() {
    for (auto mesh : GlobalData::getInstance().currentGlobalScene()->animateMeshes) {
        if (mesh->animator()) {
            mesh->animator()->updateAnimation(0);
        }
    }
}

Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::AnimationHandler::animation() {
    return m_animation;
}

Prisma::AnimationHandler::AnimationHandler() {
    auto& contextData = PrismaFunc::getInstance().contextData();
    Diligent::BufferDesc MatBufferDesc;
    MatBufferDesc.Name = "Mesh Transform Buffer";
    MatBufferDesc.Usage = Diligent::USAGE_DEFAULT;
    MatBufferDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    MatBufferDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    MatBufferDesc.ElementByteStride = sizeof(Mesh::MeshData);
    auto size = sizeof(SSBOAnimation) * Define::MAX_ANIMATION_MESHES;
    MatBufferDesc.Size = size; // Ensure enough space
    contextData.device->CreateBuffer(MatBufferDesc, nullptr, &m_animation);
}