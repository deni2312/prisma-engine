#include "Handlers/CSMHandler.h"

#include "PipelineState.h"
#include "GlobalData/PrismaFunc.h"
#include <GlobalData/GlobalShaderNames.h>

#include "glm/gtc/type_ptr.hpp"
#include "GlobalData/GlobalData.h"
#include "SceneData/MeshIndirect.h"

Prisma::CSMHandler::CSMHandler() {
    create();
    createAnimation();
}

void Prisma::CSMHandler::createShadow(unsigned int width, unsigned int height) { 
    m_width = width;
    m_height = height;
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Create window-size depth buffer
    Diligent::TextureDesc RTDepthDesc;
    RTDepthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
    RTDepthDesc.Width = m_width;
    RTDepthDesc.Height = m_height;
    RTDepthDesc.MipLevels = 1;
    RTDepthDesc.ArraySize = m_size;
    RTDepthDesc.Name = "Offscreen depth buffer CSM";
    RTDepthDesc.Format = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    RTDepthDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
    // Define optimal clear value
    RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
    RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
    RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
    contextData.device->CreateTexture(RTDepthDesc, nullptr, &m_depth);

    // Create render target views for each face
    for (int i = 0; i < m_size; ++i) {
        Diligent::TextureViewDesc DepthDesc;
        DepthDesc.ViewType = Diligent::TEXTURE_VIEW_DEPTH_STENCIL;
        DepthDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
        DepthDesc.MostDetailedMip = 0;
        DepthDesc.NumMipLevels = 1;
        DepthDesc.FirstArraySlice = i;  // Select the specific face
        DepthDesc.NumArraySlices = 1;
        Diligent::RefCntAutoPtr<Diligent::ITextureView> depth;
        m_depth->CreateView(DepthDesc, &depth);
        Diligent::RefCntAutoPtr<Diligent::ITextureView> color;
    }
}


void Prisma::CSMHandler::create() {
    // Pipeline state object encompasses configuration of all GPU stages
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "CSM Render";

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
        ShaderCI.Desc.Name = "CSM Shadow VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CSMPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    Diligent::RefCntAutoPtr<Diligent::IShader> pGS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_GEOMETRY;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "CSM Shadow GS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CSMPipeline/geometry.glsl";
        contextData.device->CreateShader(ShaderCI, &pGS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "CSM Shadow PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CSMPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - texture coordinates
        Diligent::LayoutElement{1, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{2, 0, 2, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{3, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{4, 0, 3, Diligent::VT_FLOAT32, Diligent::False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pGS = pGS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::ShaderResourceVariableDesc Vars[] =
    {
        {Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(),
         Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_GEOMETRY, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(),
         Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    Diligent::BufferDesc ShadowBuffer;
    ShadowBuffer.Name = "ShadowBuffer";
    ShadowBuffer.Usage = Diligent::USAGE_DEFAULT;
    ShadowBuffer.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    ShadowBuffer.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    ShadowBuffer.Size = sizeof(CSMShadow);
    ShadowBuffer.ElementByteStride = sizeof(CSMShadow);
    CSMShadow shadow;
    Diligent::BufferData data;
    data.DataSize = sizeof(CSMShadow);
    data.pData = &shadow;
    contextData.device->CreateBuffer(ShadowBuffer, &data, &m_shadowBuffer);

    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->
           Set(m_shadowBuffer);

    m_pso->CreateShaderResourceBinding(&m_srb, true);
    if (MeshIndirect::getInstance().modelBuffer()) {
        m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(
            MeshIndirect::getInstance().modelBuffer()->
                                        GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    }
    MeshIndirect::getInstance().addResizeHandler({"CSMMesh handler" ,[&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, MeshIndirect::MaterialView& materials) {
        m_srb.Release();
        m_pso->CreateShaderResourceBinding(&m_srb, true);
        m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(buffers->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    }});
}

void Prisma::CSMHandler::createAnimation() {
    // Pipeline state object encompasses configuration of all GPU stages
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "CSM Render";

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
    Diligent::ShaderMacro Macros[] = { {"ANIMATION", "1"} };
    ShaderCI.Macros = { Macros, _countof(Macros) };

    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "CSM Shadow VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CSMPipeline/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    Diligent::RefCntAutoPtr<Diligent::IShader> pGS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_GEOMETRY;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "CSM Shadow GS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CSMPipeline/geometry.glsl";
        contextData.device->CreateShader(ShaderCI, &pGS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "CSM Shadow PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/CSMPipeline/fragment.glsl";
        contextData.device->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    // Define vertex shader input layout
    Diligent::LayoutElement LayoutElems[] =
    {
        // Attribute 0 - vertex position
        Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, Diligent::False},
        // Attribute 1 - texture coordinates
        Diligent::LayoutElement{1, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{2, 0, 2, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{3, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{4, 0, 3, Diligent::VT_FLOAT32, Diligent::False},

        Diligent::LayoutElement{5, 0, 4, Diligent::VT_INT32, Diligent::False},

        Diligent::LayoutElement{6, 0, 4, Diligent::VT_FLOAT32, Diligent::False}
    };
    // clang-format on
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pGS = pGS;
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::ShaderResourceVariableDesc Vars[] =
    {
        {Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(),
         Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_GEOMETRY, ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str(),
         Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_psoAnimation);
    m_psoAnimation->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY,
                                            ShaderNames::CONSTANT_DIR_DATA_SHADOW.c_str())->Set(m_shadowBuffer);
    m_psoAnimation->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_ANIMATION.c_str())->
                    Set(AnimationHandler::getInstance().animation()->GetDefaultView(
                        Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    m_psoAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);
    if (MeshIndirect::getInstance().modelBufferAnimation()) {
        m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->
                        Set(MeshIndirect::getInstance().modelBufferAnimation()->GetDefaultView(
                            Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    }
    MeshIndirect::getInstance().addResizeHandler(
        {"CSMAnimation handler", [&](Diligent::RefCntAutoPtr<Diligent::IBuffer> buffers, MeshIndirect::MaterialView& materials) {
             m_srbAnimation.Release();
             m_psoAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);
             m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBufferAnimation()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
         }});

    
}

void Prisma::CSMHandler::render(const CSMData& data) {
    auto& contextData = PrismaFunc::getInstance().contextData();
    contextData.immediateContext->UpdateBuffer(m_shadowBuffer, 0, sizeof(CSMShadow), &data.shadows,
                                               Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    auto depth = m_depth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
    // Clear the back buffer
    contextData.immediateContext->SetRenderTargets(0, nullptr, depth,
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.immediateContext->ClearDepthStencil(depth, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0,
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state
    contextData.immediateContext->SetPipelineState(m_pso);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
    if (!meshes.empty()) {
        MeshIndirect::getInstance().setupBuffers();
        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(
            m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        MeshIndirect::getInstance().renderMeshes();
    }

    contextData.immediateContext->SetPipelineState(m_psoAnimation);

    auto& meshesAnimation = GlobalData::getInstance().currentGlobalScene()->animateMeshes;
    if (!meshesAnimation.empty()) {
        MeshIndirect::getInstance().setupBuffersAnimation();
        contextData.immediateContext->CommitShaderResources(m_srbAnimation,
                                                            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        MeshIndirect::getInstance().renderAnimateMeshes();
    }

    PrismaFunc::getInstance().bindMainRenderTarget();
}


Diligent::RefCntAutoPtr<Diligent::IBuffer> Prisma::CSMHandler::shadowBuffer() {
    return m_shadowBuffer; }

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::CSMHandler::shadowTexture() { return m_depth; }
