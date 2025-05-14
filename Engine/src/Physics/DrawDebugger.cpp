#ifdef JPH_DEBUG_RENDERER

#include "Physics/DrawDebugger.h"
#include "Helpers/PrismaMath.h"
#include <glm/gtx/string_cast.hpp>

#include "GlobalData/PrismaFunc.h"
#include "SceneObjects/Mesh.h"

void Prisma::DrawDebugger::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) {
    glm::vec3 from_object = JfromVec3(inFrom);
    glm::vec3 to_object = JfromVec3(inTo);
    glm::vec3 color_object;
    color_object.x = inColor.r;
    color_object.y = inColor.g;
    color_object.z = inColor.b;
    line.setColor(color_object);
    line.draw(from_object, to_object);
}

void Prisma::DrawDebugger::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor,
                                      float inHeight) {
}

void Prisma::DrawDebugger::DrawVertices(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
                                        JPH::ColorArg inColor, ECastShadow inCastShadow) {
    if (m_init) {
        /*glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_shader->use();
        m_shader->setMat4(m_modelPos, glm::mat4(1.0));
        m_shader->setVec3(m_colorPos, glm::vec3(inColor.r, inColor.g, inColor.b));
        m_vao.bind();
        std::vector<glm::vec3> v;
        v.push_back(glm::vec3(inV1.GetX(), inV1.GetY(), inV1.GetZ()));
        v.push_back(glm::vec3(inV2.GetX(), inV2.GetY(), inV2.GetZ()));
        v.push_back(glm::vec3(inV3.GetX(), inV3.GetY(), inV3.GetZ()));
        m_vbo.writeData(sizeof(glm::vec3) * v.size(), &v[0]);
        m_vao.addAttribPointer(0, 3, sizeof(glm::vec3), nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
    }
}

JPH::DebugRenderer::Batch Prisma::DrawDebugger::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) {
    auto batch = new BatchImpl;
    if (inTriangles == nullptr || inTriangleCount == 0)
        return batch;

    batch->mTriangles.assign(inTriangles, inTriangles + inTriangleCount);
    return batch;
}

JPH::DebugRenderer::Batch Prisma::DrawDebugger::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount,
                                                                    const JPH::uint32* inIndices, int inIndexCount) {
    auto batch = new BatchImpl;
    if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
        return batch;

    // Convert indexed triangle list to triangle list
    batch->mTriangles.resize(inIndexCount / 3);
    for (size_t t = 0; t < batch->mTriangles.size(); ++t) {
        Triangle& triangle = batch->mTriangles[t];
        triangle.mV[0] = inVertices[inIndices[t * 3 + 0]];
        triangle.mV[1] = inVertices[inIndices[t * 3 + 1]];
        triangle.mV[2] = inVertices[inIndices[t * 3 + 2]];
    }

    return batch;
}

void Prisma::DrawDebugger::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds,
                                        float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry,
                                        ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode) {
    if (m_init) {
        switch (inDrawMode) {
            case EDrawMode::Solid: {
                // Figure out which LOD to use
                /*const LOD* lod = inGeometry->mLODs.data();

                // Draw the batch
                auto batch = static_cast<const BatchImpl*>(lod->mTriangleBatch.GetPtr());

                std::vector<glm::vec3> vertices;

                glm::vec3 color;

                for (const Triangle& triangle : batch->mTriangles)
                {
                        vertices.push_back(JfromVec3(inModelMatrix * JPH::Vec3(triangle.mV[0].mPosition)));
                        vertices.push_back(JfromVec3(inModelMatrix * JPH::Vec3(triangle.mV[1].mPosition)));
                        vertices.push_back(JfromVec3(inModelMatrix * JPH::Vec3(triangle.mV[2].mPosition)));
                        JPH::Color modelColor = inModelColor * triangle.mV[0].mColor;
                        color = glm::vec3(modelColor.r, modelColor.g, modelColor.b);
                }

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                m_shader->use();
                m_shader->setMat4(m_modelPos, glm::mat4(1.0));
                m_shader->setVec3(m_colorPos, color);
                m_vao.bind();
                m_vbo.writeData(sizeof(glm::vec3) * vertices.size(), &vertices[0]);
                m_vao.addAttribPointer(0, 3, sizeof(glm::vec3), nullptr);
                glDrawArrays(GL_TRIANGLES, 0, vertices.size());
                glBindVertexArray(0);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
                break;
            }
        }
    }
}

void Prisma::DrawDebugger::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3,
                                        JPH::ColorArg inColor, ECastShadow inCastShadow) {
}

void Prisma::DrawDebugger::init() {
    if (!m_init) {
        m_init = true;
        // Pipeline state object encompasses configuration of all GPU stages
        auto& contextData = PrismaFunc::getInstance().contextData();

  /*      Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSOCreateInfo.PSODesc.Name = "Debug Physics";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

        // clang-format off
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 0;
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_UNKNOWN;
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.DepthClipEnable = Diligent::False;
    Diligent::ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;


    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Debug Physics VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DebugPhysicsPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Debug Physics PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/DebugPhysicsPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
    };
        // clang-format on
        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        Diligent::BufferDesc CBDesc;
        CBDesc.Name = "Debug Physics Buffer";
        CBDesc.Size = sizeof(DebugPhysicsBufferData);
        CBDesc.Usage = Diligent::USAGE_DYNAMIC;
        CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        contextData.device->CreateBuffer(CBDesc, nullptr, &m_buffer);

        contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);
        m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "MeshData")->Set(m_buffer);

        m_pso->CreateShaderResourceBinding(&m_srb, true);*/
    }
}

Prisma::DrawDebugger::DrawDebugger() {
    Initialize();
}
#endif