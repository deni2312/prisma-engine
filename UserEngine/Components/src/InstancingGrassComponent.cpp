#include "../include/InstancingGrassComponent.h"
#include "GlobalData/PrismaFunc.h"
#include "Pipelines/PipelineHandler.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Handlers/LightHandler.h"
#include "Pipelines/PipelineLUT.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineDiffuseIrradiance.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>

using namespace Diligent;

Prisma::InstancingGrassComponent::InstancingGrassComponent() : RenderComponent{} { name("InstancingGrass"); }

void Prisma::InstancingGrassComponent::start() { 
	Component::start(); 
	auto& contextData = PrismaFunc::getInstance().contextData();
    m_counter.start();

    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Forward Pipeline Grass Instancing Renderer";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PipelineHandler::getInstance().textureFormat();
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
        // clang-format on

        ShaderCreateInfo ShaderCI;

        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_GLSL;
        ShaderCI.CompileFlags |= SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;

        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Pack matrices in row-major order

        // In this tutorial, we will load shaders from file. To be able to do that,
        // we need to create a shader source stream factory
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        contextData.engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Forward VS Instancing Grass Renderer";
            ShaderCI.FilePath = "../../../UserEngine/Shaders/ForwardGrassInstancingPipeline/vertex.glsl";
            contextData.device->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Forward PS Instancing Grass Renderer";
            ShaderCI.FilePath = "../../../UserEngine/Shaders/ForwardGrassInstancingPipeline/fragment.glsl";
            contextData.device->CreateShader(ShaderCI, &pPS);
        }

        Diligent::BufferDesc CBDesc;
        CBDesc.Name = "Time Buffer";
        CBDesc.Size = sizeof(glm::vec4);
        CBDesc.Usage = Diligent::USAGE_DYNAMIC;
        CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        contextData.device->CreateBuffer(CBDesc, nullptr, &m_timeBuffer);

        // clang-format off
        // Define vertex shader input layout
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 3, VT_FLOAT32, False},

            LayoutElement{2, 0, 2, VT_FLOAT32, False},

            LayoutElement{3, 0, 3, VT_FLOAT32, False},

            LayoutElement{4, 0, 3, VT_FLOAT32, False}
        };
        // clang-format on
        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        std::string samplerClampName = "textureClamp_sampler";
        std::string samplerRepeatName = "textureRepeat_sampler";

        PipelineResourceDesc Resources[] = {
            {SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_INDEX_OPAQUE.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, "TimeBuffer", 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_OMNI_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_SPOT_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str(), Define::MAX_OMNI_SHADOW, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIR_SHADOW.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str(), Define::MAX_MESHES, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

            {SHADER_TYPE_PIXEL, samplerClampName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, samplerRepeatName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

            {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LUT.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_PREFILTER.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_IRRADIANCE.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

        };

    PipelineResourceSignatureDesc ResourceSignDesc;
    ResourceSignDesc.NumResources = _countof(Resources);
    ResourceSignDesc.Resources = Resources;



        // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };

    SamplerDesc SamLinearRepeatDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
        TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP
    };

    RefCntAutoPtr<ISampler> samplerClamp;
    RefCntAutoPtr<ISampler> samplerRepeat;

    contextData.device->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);

    IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignature };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);


    contextData.device->CreatePipelineState(PSOCreateInfo, &m_pso);
    contextData.device->CreateSampler(SamLinearClampDesc, &samplerClamp);
    contextData.device->CreateSampler(SamLinearRepeatDesc, &samplerRepeat);


    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_VERTEX, "TimeBuffer")->Set(m_timeBuffer);

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->Set(CSMHandler::getInstance().shadowBuffer());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_SPOT_DATA.c_str())->Set(LightHandler::getInstance().spotLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_SPOT_DATA.c_str())->Set(LightHandler::getInstance().spotLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA.c_str())->Set(LightHandler::getInstance().dirLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS.c_str())->Set(LightHandler::getInstance().clusters().clusters->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str())->Set(LightHandler::getInstance().clusters().clustersData);

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LIGHT_SIZES.c_str())->Set(LightHandler::getInstance().lightSizes());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LUT.c_str())->Set(PipelineLUT::getInstance().lutTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));

    IDeviceObject* samplerDeviceClamp = samplerClamp;
    IDeviceObject* samplerDeviceRepeat = samplerRepeat;

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerClampName.c_str())->Set(samplerDeviceClamp);
    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, samplerRepeatName.c_str())->Set(samplerDeviceRepeat);

    // Create a shader resource binding object and bind all static resources in it
    m_pResourceSignature->CreateShaderResourceBinding(&m_srbOpaque, true);

    m_updateData = [&](RefCntAutoPtr<IShaderResourceBinding>& srb)
        {
            auto buffers = MeshIndirect::getInstance().modelBuffer();
            auto materials = MeshIndirect::getInstance().textureViews();
            auto status = MeshIndirect::getInstance().statusBuffer();
            srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&srb, true);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str())->SetArray(LightHandler::getInstance().omniData().data(), 0, LightHandler::getInstance().omniData().size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIR_SHADOW.c_str())->Set(LightHandler::getInstance().dirShadowData());
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIFFUSE_TEXTURE.c_str())->Set(m_mesh->material()->diffuse()[0].texture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_NORMAL_TEXTURE.c_str())->Set(m_mesh->material()->normal()[0].texture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_ROUGHNESS_METALNESS_TEXTURE.c_str())->Set(m_mesh->material()->roughnessMetalness()[0].texture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            srb->GetVariableByName(SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(m_modelsBuffer->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

            if (status) {
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str())->Set(status->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            }
            if (PipelineSkybox::getInstance().isInit()) {
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_PREFILTER.c_str())->Set(PipelinePrefilter::getInstance().prefilterTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_IRRADIANCE.c_str())->Set(PipelineDiffuseIrradiance::getInstance().irradianceTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            }
        };

    m_updateData(m_srbOpaque);

    //CreateMSAARenderTarget();
    MeshIndirect::getInstance().addResizeHandler({std::to_string(uuid()),[&](RefCntAutoPtr<IBuffer> buffers, MeshIndirect::MaterialView& materials)
        {
            m_updateData(m_srbOpaque);
        }});
    PipelineSkybox::getInstance().addUpdate({std::to_string(uuid()),[&]()
        {
            m_updateData(m_srbOpaque);
        }});
    LightHandler::getInstance().addLightHandler({std::to_string(uuid()),[&]()
        {
            m_updateData(m_srbOpaque);
        }});
}

void Prisma::InstancingGrassComponent::models(const std::vector<Mesh::MeshData>& models) { 
        m_models = models;
        auto& contextData = PrismaFunc::getInstance().contextData();
        if (m_modelsBuffer) {
            m_modelsBuffer->Release();
        }

        Diligent::BufferDesc InstancingDesc;
        InstancingDesc.Name = "Instancing Models Buffer";
        InstancingDesc.Usage = Diligent::USAGE_DEFAULT;
        InstancingDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
        InstancingDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
        InstancingDesc.ElementByteStride = sizeof(Mesh::MeshData);
        InstancingDesc.Size = m_models.size() * sizeof(Mesh::MeshData);
        BufferData data;
        data.pData=models.data();
        data.DataSize=InstancingDesc.Size;

        contextData.device->CreateBuffer(InstancingDesc, &data, &m_modelsBuffer);
}

void Prisma::InstancingGrassComponent::updateModels(const std::vector<Mesh::MeshData>& models)
{
    auto& contextData = PrismaFunc::getInstance().contextData();
    contextData.immediateContext->UpdateBuffer(m_modelsBuffer, 0, m_models.size() * sizeof(Mesh::MeshData), models.data(),Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Prisma::InstancingGrassComponent::mesh(std::shared_ptr<Prisma::Mesh> mesh)
{
    m_mesh=mesh;
    auto vertices=m_mesh->verticesData();

    if (m_bufferData.vBuffer) {
        m_bufferData.vBuffer.Release();
        m_bufferData.iBuffer.Release();
    }

    // Create vertex buffer
    Diligent::BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Vertices Data";
    VertBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
    VertBuffDesc.Size = sizeof(Prisma::Mesh::Vertex) * vertices.vertices.size();

    Diligent::BufferData VBData;
    VBData.pData = vertices.vertices.data();
    VBData.DataSize = VertBuffDesc.Size;

    PrismaFunc::getInstance().contextData().device->CreateBuffer(
        VertBuffDesc, &VBData, &m_bufferData.vBuffer);

    // Create index buffer
    Diligent::BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "Index Data";
    IndBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
    IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    IndBuffDesc.Size = sizeof(unsigned int) * vertices.indices.size();

    Diligent::BufferData IBData;
    IBData.pData = vertices.indices.data();
    IBData.DataSize = IndBuffDesc.Size;
    m_bufferData.iBufferSize = vertices.indices.size();
    PrismaFunc::getInstance().contextData().device->CreateBuffer(
    IndBuffDesc, &IBData, &m_bufferData.iBuffer);
}

void Prisma::InstancingGrassComponent::updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) { 
	RenderComponent::updatePostRender(texture, depth);
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::MapHelper<glm::vec4> time(contextData.immediateContext, m_timeBuffer,
                                                           Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

    time->r=m_counter.duration_seconds();

    contextData.immediateContext->SetPipelineState(m_pso);

    // Bind vertex and index buffers
    constexpr Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pBuffs[] = {m_bufferData.vBuffer};
    contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    contextData.immediateContext->SetIndexBuffer(m_bufferData.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set texture SRV in the SRB
    contextData.immediateContext->CommitShaderResources(m_srbOpaque, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs DrawAttrs; // This is an indexed draw call
    DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
    DrawAttrs.NumIndices = m_bufferData.iBufferSize;
    DrawAttrs.NumInstances=m_models.size();
    // Verify the state of vertex and index buffers
    DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;

    contextData.immediateContext->DrawIndexed(DrawAttrs);
}

void Prisma::InstancingGrassComponent::destroy()
{
    MeshIndirect::getInstance().removeResizeHandler(std::to_string(uuid()));
    PipelineSkybox::getInstance().removeUpdate(std::to_string(uuid()));
    LightHandler::getInstance().removeLightHandler(std::to_string(uuid()));
    Component::destroy();
}

void Prisma::InstancingGrassComponent::ui() { Component::ui(); }