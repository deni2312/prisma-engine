#pragma once
#include <vector>
#include <memory>
#include "../SceneObjects/AnimatedMesh.h"
#include "../SceneObjects/Camera.h"
#include "../Components/MaterialComponent.h"
#include "../GlobalData/InstanceData.h"
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"

namespace Prisma {
struct DrawElementsIndirectCommand {
    unsigned int count;
    unsigned int instanceCount;
    unsigned int firstIndex;
    unsigned int baseVertex;
    unsigned int baseInstance;
};

class MeshIndirect : public InstanceData<MeshIndirect> {
public:
    struct MaterialView {
        std::vector<Diligent::IDeviceObject*> diffuse;
        std::vector<Diligent::IDeviceObject*> normal;
        std::vector<Diligent::IDeviceObject*> rm;
        std::vector<Diligent::IDeviceObject*> specular;
    };

    //std::shared_ptr<VAO> vao();

    //std::shared_ptr<VBO> vbo();

    //std::shared_ptr<EBO> ebo();

    Mesh::VerticesData& verticesData();

    void load();

    void init();

    void add(unsigned int add);

    void remove(unsigned int remove);

    void updateModels(int model);

    void addAnimate(unsigned int add);

    void removeAnimate(unsigned int remove);

    void updateModelsAnimate(int model);

    void renderMeshes() const;
    void renderMeshesOpaque() const;
    void renderMeshesTransparent() const;
    void renderMeshesCopy() const;
    void renderAnimateMeshes() const;

    void update();
    void updateSize();
    void updateModels();
    void updateModelsAnimation();
    void updateTextureSize(bool update = true);
    void updateStatus(bool update = true);
    void updateStatusAnimation(bool update = true);

    void setupBuffers();
    void setupBuffersAnimation();

    void addResizeHandler(std::pair<std::string, std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>, MaterialView&)>> resizeHandler);
    void removeResizeHandler(const std::string& resizeHandler);

    MeshIndirect();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> modelBuffer();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> modelBufferAnimation();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer();

    MaterialView& textureViews();
    MaterialView& textureViewsAnimation();

    Diligent::DrawIndexedIndirectAttribs commandsBuffer();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> statusBuffer();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> statusBufferAnimation();

    Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBufferOpaque();
    Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBufferTransparent();

private:
    //BINDING DATA

    MaterialView m_textureViews;
    MaterialView m_textureViewsAnimation;

    Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indirectBufferAll;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indirectBufferOpaque;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indirectBufferTransparent;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_statusBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_modelBuffer;
    Diligent::DrawIndexedIndirectAttribs m_commandsBufferAll;
    Diligent::DrawIndexedIndirectAttribs m_commandsBufferOpaque;
    Diligent::DrawIndexedIndirectAttribs m_commandsBufferTransparent;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vBuffer;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iBuffer;

    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indexBufferOpaque;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indexBufferTransparent;


    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_vBufferAnimation;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_iBufferAnimation;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_indirectBufferAnimation;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_statusBufferAnimation;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> m_modelBufferAnimation;
    Diligent::DrawIndexedIndirectAttribs m_commandsBufferAnimation;

    void resizeModels(std::vector<Mesh::MeshData>& models);
    void resizeModelsAnimation(std::vector<Mesh::MeshData>& models);

    void createMeshBuffer();
    void createMeshAnimationBuffer();

    std::map<std::string, std::function<void(Diligent::RefCntAutoPtr<Diligent::IBuffer>, MaterialView&)>> m_resizeHandler;

    //INDIRECT INDEX

    unsigned int m_currentIndex;
    unsigned int m_currentVertex;

    unsigned int m_indirectDrawAnimation;
    unsigned int m_currentIndexAnimation;
    unsigned int m_currentVertexAnimation;

    //CURRENT CACHE DATA

    const unsigned int m_cacheSize = 1000;

    uint64_t m_currentVertexMax = 0;
    uint64_t m_currentIndexMax = 0;

    uint64_t m_currentVertexMaxAnimation = 0;
    uint64_t m_currentIndexMaxAnimation = 0;

    //VERTICES DATA

    Mesh::VerticesData m_verticesData;


    AnimatedMesh::AnimateVerticesData m_verticesDataAnimation;
    std::vector<DrawElementsIndirectCommand> m_drawCommandsAnimation;

    void updateAnimation();

    std::vector<unsigned int> m_cacheAddAnimate;
    std::vector<unsigned int> m_cacheRemoveAnimate;


    std::vector<unsigned int> m_cacheAdd;
    std::vector<unsigned int> m_cacheRemove;

    void updateIndirectBuffer();
    void updateIndirectBufferAnimation();


    unsigned int m_sizeLocation;
    unsigned int m_indicesCopyLocation;

    std::map<int, int> m_updateModels;
    std::map<int, int> m_updateModelsAnimate;

    struct CameraData {
        float zNear;
        float zFar;
        float fovY;
        float aspect;
    };

    struct StatusData {
        unsigned int status;
        int plainMaterial;
        int transparent = 0;
        int isSpecular = 0;
        // Refraction cube properties
        glm::vec3 GlassReflectionColorMask = glm::vec3(0.22f, 0.83f, 0.93f);
        float GlassAbsorption = 0.5;
        glm::vec4 GlassMaterialColor = glm::vec4(1);
        glm::vec2 GlassIndexOfRefraction = glm::vec2(1.5f, 1.02f); // min and max IOR
        int GlassEnableDispersion = 0;
        unsigned int DispersionSampleCount = 4; // 1..16
        float metalness = 0;
        float roughness = 0;
        float emission = 0;
        float padding;
    };

    void updateTextureData();
    void updateTextureDataAnimation();
    void updatePso();
};
}