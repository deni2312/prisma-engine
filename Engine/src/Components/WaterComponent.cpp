#include "Components/WaterComponent.h"

#include <GlobalData/GlobalShaderNames.h>
#include <Helpers/NoiseGenerator.h>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>

#include "GlobalData/PrismaFunc.h"
#include "Helpers/Logger.h"
#include "Helpers/PrismaRender.h"
#include "Helpers/SettingsLoader.h"
#include "Pipelines/PipelineHandler.h"
#include "Postprocess/Postprocess.h"
#include "TextureLoader/interface/TextureLoader.h"
#include "TextureLoader/interface/TextureUtilities.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Handlers/LightHandler.h"
#include "Pipelines/PipelineLUT.h"
#include "Pipelines/PipelinePrefilter.h"
#include "Pipelines/PipelineDiffuseIrradiance.h"
#include "engine.h"

using namespace Diligent;

Prisma::WaterComponent::WaterComponent() { name("WaterComponent"); }

void Prisma::WaterComponent::ui() {
    Prisma::Component::ui();
    ComponentType componentSpeed;
    componentSpeed = std::make_tuple(TYPES::FLOAT, "Speed", &m_waterConstants.waveSpeed);
    addGlobal({componentSpeed, false});

    ComponentType componentAmplitude;
    componentAmplitude = std::make_tuple(TYPES::FLOAT, "Amplitude", &m_waterConstants.waveAmplitude);
    addGlobal({componentAmplitude, false});

    ComponentType componentFrequency;
    componentFrequency = std::make_tuple(TYPES::FLOAT, "Frequency", &m_waterConstants.waveFrequency);
    addGlobal({componentFrequency, false});


    ComponentType componentRun;
    m_run = [&]() {
        if (!isStart()) {
            start();
        }
    };
    componentRun = std::make_tuple(TYPES::BUTTON, "Run UI", &m_run);
    addGlobal({componentRun, false});
}

void Prisma::WaterComponent::update() {  }

void Prisma::WaterComponent::start() {
    Component::start();
    m_waterConstants.size = m_resolution;
    createPlaneMesh();
    createCompute();
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Water Pipeline Instancing Renderer";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 3;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[1] = Prisma::PipelineHandler::getInstance().textureFormat();
    PSOCreateInfo.GraphicsPipeline.RTVFormats[2] = Prisma::PipelineHandler::getInstance().textureFormat();
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;

    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;
    ShaderCI.CompileFlags |= Diligent::SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;

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
        ShaderCI.Desc.Name = "Water VS Instancing Renderer";
        ShaderCI.FilePath = "../../../Engine/Shaders/WaterPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Water PS Instancing Renderer";
        ShaderCI.FilePath = "../../../Engine/Shaders/WaterPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        LayoutElement{0, 0, 4, VT_FLOAT32, False},
        // Attribute 1 - texture coordinates
        LayoutElement{1, 0, 4, VT_FLOAT32, False},

        LayoutElement{2, 0, 4, VT_FLOAT32, False},

        LayoutElement{3, 0, 4, VT_FLOAT32, False},

        LayoutElement{4, 0, 4, VT_FLOAT32, False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "WaterModel Constants";
    CBDesc.Size = sizeof(Prisma::Mesh::MeshData);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    contextData.device->CreateBuffer(CBDesc, nullptr, &m_modelConstant);

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    std::string samplerClampName = "textureClamp_sampler";
    std::string samplerRepeatName = "textureRepeat_sampler";

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Water Reflection";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = PipelineHandler::getInstance().textureFormat();
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0.350f;
    RTColorDesc.ClearValue.Color[1] = 0.350f;
    RTColorDesc.ClearValue.Color[2] = 0.350f;
    RTColorDesc.ClearValue.Color[3] = 1.f;
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_reflection);
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_finalReflection);
    GlobalData::getInstance().addGlobalTexture({m_finalReflection, "Reflection Final Texture"});

    PipelineResourceDesc Resources[] = {
        {SHADER_TYPE_VERTEX, "ModelConstant", 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LIGHT_SIZES.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_OMNI_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS.c_str(), 1, SHADER_RESOURCE_TYPE_BUFFER_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_CLUSTERS_DATA.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(), 1, SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str(), Define::MAX_OMNI_SHADOW, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIR_SHADOW.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, samplerClampName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, samplerRepeatName.c_str(), 1, SHADER_RESOURCE_TYPE_SAMPLER, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

        {SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_LUT.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_PREFILTER.c_str(), 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {SHADER_TYPE_PIXEL, "skybox", 1, SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
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

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_VERTEX, "ModelConstant")->Set(m_modelConstant);

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->Set(CSMHandler::getInstance().shadowBuffer());

    m_pResourceSignature->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_OMNI_DATA.c_str())->Set(LightHandler::getInstance().omniLights()->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));

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
    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

    m_updateData = [&](RefCntAutoPtr<IShaderResourceBinding>& srb)
        {
            auto buffers = MeshIndirect::getInstance().modelBuffer();
            auto materials = MeshIndirect::getInstance().textureViews();
            auto status = MeshIndirect::getInstance().statusBuffer();
            srb.Release();
            m_pResourceSignature->CreateShaderResourceBinding(&srb, true);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_OMNI_DATA_SHADOW.c_str())->SetArray(LightHandler::getInstance().omniData().data(), 0, LightHandler::getInstance().omniData().size(), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_DIR_SHADOW.c_str())->Set(LightHandler::getInstance().dirShadowData());

            if (status) {
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_STATUS.c_str())->Set(status->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
            }

            if (PipelineSkybox::getInstance().isInit()) {
                auto skybox=PipelineSkybox::getInstance().skybox();
                srb->GetVariableByName(SHADER_TYPE_PIXEL, "skybox")->Set(skybox->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_PREFILTER.c_str())->Set(PipelinePrefilter::getInstance().prefilterTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
                srb->GetVariableByName(SHADER_TYPE_PIXEL, ShaderNames::MUTABLE_IRRADIANCE.c_str())->Set(PipelineDiffuseIrradiance::getInstance().irradianceTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
            }
        };

    m_updateData(m_srb);

    //CreateMSAARenderTarget();
    MeshIndirect::getInstance().addResizeHandler({std::to_string(uuid()),[&](RefCntAutoPtr<IBuffer> buffers, MeshIndirect::MaterialView& materials)
        {
            m_updateData(m_srb);
        }});
    PipelineSkybox::getInstance().addUpdate({std::to_string(uuid()),[&]()
        {
            m_updateData(m_srb);
        }});
    LightHandler::getInstance().addLightHandler({std::to_string(uuid()),[&]()
        {
            m_updateData(m_srb);
        }});
    GlobalData::getInstance().addGlobalTexture({m_reflection, "Water Reflection Texture"});

    createReflection();
}

void Prisma::WaterComponent::destroy() { 
    
    MeshIndirect::getInstance().removeResizeHandler(std::to_string(uuid()));
    PipelineSkybox::getInstance().removeUpdate(std::to_string(uuid()));
    LightHandler::getInstance().removeLightHandler(std::to_string(uuid()));
    Component::destroy();
}

void Prisma::WaterComponent::updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {

    RenderComponent::updatePostRender(texture, depth);

    if (Prisma::Engine::getInstance().engineSettings().pipeline == Prisma::EngineSettings::Pipeline::DEFERRED_FORWARD) {

        computeWater();

        auto pDSV = PipelineHandler::getInstance().textureData().pDepthDSV->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);

        auto& contextData = PrismaFunc::getInstance().contextData();
        ITextureView* textures[] = {texture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET), m_reflection->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),Prisma::PipelineHandler::getInstance().deferredForward()->positionTexture()->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET)};

        // Clear the back buffer
        contextData.immediateContext->SetRenderTargets(3, textures, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        contextData.immediateContext->ClearRenderTarget(m_reflection->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET), value_ptr(glm::vec4(0)), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        contextData.immediateContext->SetPipelineState(m_pso);
    
        Diligent::MapHelper<Prisma::Mesh::MeshData> m_modelConstant(contextData.immediateContext, m_modelConstant, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

        auto finalMatrix=parent()->finalMatrix();

        *m_modelConstant = {finalMatrix,glm::transpose(glm::inverse(finalMatrix))};
        // Bind vertex and index buffers
        constexpr Diligent::Uint64 offset = 0;
        Diligent::IBuffer* pBuffs[] = {m_vBuffer};
        contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.immediateContext->SetIndexBuffer(m_iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs DrawAttrs; // This is an indexed draw call
        DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
        DrawAttrs.NumIndices = m_iBufferSize;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;

        contextData.immediateContext->DrawIndexed(DrawAttrs);
        
        renderReflection();

        auto mainTexture=texture->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);

        contextData.immediateContext->SetRenderTargets(1, &mainTexture, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}

void Prisma::WaterComponent::updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {
}

void Prisma::WaterComponent::nodePosition(std::shared_ptr<Prisma::Node> node)
{
    m_node=node;
}

void Prisma::WaterComponent::radius(float radius)
{
    m_waterConstants.radius=radius;
}

void Prisma::WaterComponent::createPlaneMesh()
{

    std::vector<AlignedVertex> vertices;
    std::vector<unsigned int> indices;

    float halfWidth = m_length / 2.0f;
    float step = m_length / (m_resolution - 1);

    for (int z = 0; z < m_resolution; ++z)
    {
        for (int x = 0; x < m_resolution; ++x)
        {
            AlignedVertex v;

            float xpos = -halfWidth + x * step;
            float zpos = -halfWidth + z * step;

            // Set position
            v.position = glm::vec4(xpos, 0.0f, zpos,1);

            // Set normal pointing up
            v.normal = glm::vec4(0.0f, 1.0f, 0.0f,1);

            // Tangent along +X, bitangent along +Z
            v.tangent = glm::vec4(1.0f, 0.0f, 0.0f,1);
            v.bitangent = glm::vec4(0.0f, 0.0f, 1.0f,1);

            // Texture coordinates from 0 to 1
            v.texCoords = glm::vec4((float)x / (m_resolution - 1), (float)z / (m_resolution - 1),1,1);

            vertices.push_back(v);
        }
    }

    // Generate indices (two triangles per quad)
    for (int z = 0; z < m_resolution - 1; ++z)
    {
        for (int x = 0; x < m_resolution - 1; ++x)
        {
            int topLeft = z * m_resolution + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * m_resolution + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }


    // Create vertex buffer
    Diligent::BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Vertices Data";
    VertBuffDesc.Usage = Diligent::USAGE_DEFAULT;
    VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_UNORDERED_ACCESS;
    VertBuffDesc.Size = sizeof(AlignedVertex) * vertices.size();
    VertBuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    VertBuffDesc.ElementByteStride = sizeof(AlignedVertex);

    Diligent::BufferData VBData;
    VBData.pData = vertices.data();
    VBData.DataSize = VertBuffDesc.Size;

    PrismaFunc::getInstance().contextData().device->CreateBuffer(
        VertBuffDesc, &VBData, &m_vBuffer);

    // Create index buffer
    Diligent::BufferDesc IndBuffDesc;
    IndBuffDesc.Name = "Index Data";
    IndBuffDesc.Usage = Diligent::USAGE_DEFAULT;
    IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
    IndBuffDesc.Size = sizeof(unsigned int) * indices.size();

    Diligent::BufferData IBData;
    IBData.pData = indices.data();
    IBData.DataSize = IndBuffDesc.Size;
    m_iBufferSize = indices.size();
    PrismaFunc::getInstance().contextData().device->CreateBuffer(
        IndBuffDesc, &IBData, &m_iBuffer);
}

void Prisma::WaterComponent::createCompute()
{
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    // Create a shader source stream factory to load shaders from files.
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    contextData.engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

    Diligent::RefCntAutoPtr<Diligent::IShader> pResetParticleListsCS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_COMPUTE;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Water CS";
        ShaderCI.FilePath = "../../../Engine/Shaders/WaterPipeline/water.glsl";
        contextData.device->CreateShader(ShaderCI, &pResetParticleListsCS);
    }

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "WaterComponent Constants";
    CBDesc.Size = sizeof(WaterConstants);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
    contextData.device->CreateBuffer(CBDesc, nullptr, &m_constants);

    Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
    Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

    // This is a compute pipeline
    PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

    PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
	// clang-format off
	Diligent::ShaderResourceVariableDesc Vars[] =
	{
		{Diligent::SHADER_TYPE_COMPUTE, "WaterMesh", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_COMPUTE, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
	};
    // clang-format on
    PSODesc.ResourceLayout.Variables = Vars;
    PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    PSODesc.Name = "Cluster";
    PSOCreateInfo.pCS = pResetParticleListsCS;
    contextData.device->CreateComputePipelineState(PSOCreateInfo, &m_psoCompute);
    m_psoCompute->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "WaterMesh")->Set(m_vBuffer->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS));
    m_psoCompute->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Constants")->Set(m_constants);
    m_psoCompute->CreateShaderResourceBinding(&m_srbCompute, true);
    m_counter.start();
}

void Prisma::WaterComponent::createReflection() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Water Reflection Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = PipelineHandler::getInstance().textureFormat();

    // Set depth buffer format which is the format of the swap chain's back buffer
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_GLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    auto settings = Prisma::SettingsLoader::getInstance().getSettings();

    std::string widthStr = std::to_string(static_cast<int>(settings.width));
    std::string heightStr = std::to_string(static_cast<int>(settings.height));

    Diligent::ShaderMacro Macros[] = {{"WIDTH", widthStr.c_str()}, {"HEIGHT", heightStr.c_str()}};
    ShaderCI.Macros = {Macros, _countof(Macros)};
    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Water Reflection VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/WaterReflectionPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Water Reflection PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/WaterReflectionPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
	Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - texture coordinates
        Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, Diligent::False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

   Diligent::ShaderResourceVariableDesc Vars[] = {
        {Diligent::SHADER_TYPE_PIXEL, "screenTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        //{Diligent::SHADER_TYPE_PIXEL, "waterMaskTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        //{Diligent::SHADER_TYPE_PIXEL, "positionTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        //{Diligent::SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
	// Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
	Diligent::SamplerDesc SamLinearClampDesc
    {
	    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
	    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP
    };

	Diligent::ImmutableSamplerDesc ImtblSamplers[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "screenTexture", SamLinearClampDesc}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_psoReflection);

    m_psoReflection->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "screenTexture")->Set(Prisma::PipelineHandler::getInstance().textureData().pColorRTV->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    m_psoReflection->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "normalTexture")->Set(Prisma::PipelineHandler::getInstance().deferredForward()->normalTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    m_psoReflection->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "waterMaskTexture")->Set(m_reflection->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    m_psoReflection->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "positionTexture")->Set(Prisma::PipelineHandler::getInstance().deferredForward()->positionTexture()->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    m_psoReflection->GetStaticVariableByName(SHADER_TYPE_PIXEL, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());
    
    m_psoReflection->CreateShaderResourceBinding(&m_srbReflection, true);

    m_blit = std::make_unique<Blit>(m_finalReflection);
}

void Prisma::WaterComponent::renderReflection() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    auto color = m_finalReflection->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    contextData.immediateContext->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(color, value_ptr(glm::vec4(0)), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->SetPipelineState(m_psoReflection);

    auto quadBuffer = PrismaRender::getInstance().quadBuffer();

    // Bind vertex and index buffers
    constexpr Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
    contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    contextData.immediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Set texture SRV in the SRB
    contextData.immediateContext->CommitShaderResources(m_srbReflection, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
    DrawAttrs.IndexType = Diligent::VT_UINT32;  // Index type
    DrawAttrs.NumIndices = quadBuffer.iBufferSize;
    // Verify the state of vertex and index buffers
    DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    contextData.immediateContext->DrawIndexed(DrawAttrs);

    m_blit->render(PipelineHandler::getInstance().textureData().pColorRTV);
}

void Prisma::WaterComponent::computeWater() {
    auto& contextData = PrismaFunc::getInstance().contextData();
    if (m_node) {
        m_waterConstants.touchPosition = m_node->finalMatrix()[3];
    }

    m_waterConstants.time.r = m_counter.duration_seconds();



    Diligent::MapHelper<WaterConstants> waterData(contextData.immediateContext, m_constants, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

    *waterData = m_waterConstants;

    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = m_resolution;
    DispatAttribs.ThreadGroupCountY = m_resolution;
    DispatAttribs.ThreadGroupCountZ = 1;
    contextData.immediateContext->SetPipelineState(m_psoCompute);
    contextData.immediateContext->CommitShaderResources(m_srbCompute, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->DispatchCompute(DispatAttribs);
}
