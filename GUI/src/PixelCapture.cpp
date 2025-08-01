#include "../include/PixelCapture.h"
#include "Helpers/SettingsLoader.h"
#include "Helpers/PrismaRender.h"
#include "SceneData/MeshIndirect.h"
#include <glm/gtx/string_cast.hpp>
#include "GlobalData/GlobalData.h"
#include "Handlers/MeshHandler.h"
#include "GlobalData/GlobalShaderNames.h"
#include "glm/gtc/type_ptr.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

//static std::shared_ptr<Prisma::Shader> shader = nullptr;
//static std::shared_ptr<Prisma::Shader> shaderAnimation = nullptr;
//static std::shared_ptr<Prisma::Shader> shaderData = nullptr;

Prisma::GUI::PixelCapture::PixelCapture() {
    createDrawPipeline();
    createDrawAnimationPipeline();
    createScalePipeline();
}

std::shared_ptr<Prisma::Mesh> Prisma::GUI::PixelCapture::capture(glm::vec2 position, const glm::mat4& model) {
    /*m_fbo->bind();
    GLfloat bkColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor);

    float color = bkColor[3];

    bkColor[3] = 0.2;

    glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bkColor[3] = color;

    glClearColor(bkColor[0], bkColor[1], bkColor[2], bkColor[3]);

    shader->use();

    MeshIndirect::getInstance().renderMeshesCopy();

    shaderAnimation->use();

    MeshIndirect::getInstance().renderAnimateMeshes();

    m_fbo->unbind();


    m_fboData->bind();
    GLfloat bkColor1[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, bkColor1);

    float color1 = bkColor[3];

    bkColor1[3] = 0.2;

    glClearColor(bkColor1[0], bkColor1[1], bkColor1[2], bkColor1[3]);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bkColor1[3] = color1;

    glClearColor(bkColor1[0], bkColor1[1], bkColor1[2], bkColor1[3]);

    shaderData->use();
    shaderData->setMat4(m_modelLocation, model);
    shaderData->setInt64(m_textureLocation, m_fbo->texture());
    //Prisma::PrismaRender::getInstance().renderQuad();

    glFlush();
    glFinish();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char data[4];

    glReadPixels(position.x, position.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

    m_fboData->unbind();


    uint32_t encodedUUID = (data[0] << 16) | (data[1] << 8) | data[2];
    if (data[3] < 0.1)
    {
            if (encodedUUID < Prisma::GlobalData::getInstance().currentGlobalScene()->meshes.size() && encodedUUID >= 0)
            {
                    return Prisma::GlobalData::getInstance().currentGlobalScene()->meshes[encodedUUID];
            }
    }
    else if (data[3] < 255 && data[3] > 0)
    {
            return nullptr;
    }
    else
    {
            if (encodedUUID < Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes.size() && encodedUUID >=
                    0)
            {
                    return Prisma::GlobalData::getInstance().currentGlobalScene()->animateMeshes[encodedUUID];
            }
    }
    return nullptr;*/
    auto& contextData = PrismaFunc::getInstance().contextData();
    auto rt = m_pRTColor->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
    auto rtDepth = m_pRTDepth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
    // Clear the back buffer
    contextData.immediateContext->SetRenderTargets(1, &rt, rtDepth,
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(rt, value_ptr(m_clearColor),
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearDepthStencil(rtDepth, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0,
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state
    contextData.immediateContext->SetPipelineState(m_pso);
    auto& meshes = GlobalData::getInstance().currentGlobalScene()->meshes;
    if (!meshes.empty() && MeshIndirect::getInstance().commandsBuffer().pAttribsBuffer) {
        MeshIndirect::getInstance().setupBuffers();
        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(
            m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        MeshIndirect::getInstance().renderMeshes();
    }

    // Set the pipeline state
    contextData.immediateContext->SetPipelineState(m_psoAnimation);
    auto& meshesAnimation = GlobalData::getInstance().currentGlobalScene()->animateMeshes;
    if (!meshesAnimation.empty() && MeshIndirect::getInstance().commandsBuffer().pAttribsBuffer) {
        MeshIndirect::getInstance().setupBuffersAnimation();
        // Set texture SRV in the SRB
        contextData.immediateContext->CommitShaderResources(m_srbAnimation,
                                                            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        MeshIndirect::getInstance().renderAnimateMeshes();
    }
    contextData.immediateContext->SetRenderTargets(0, nullptr, nullptr,
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

    drawModel(model);
    auto settings = SettingsLoader::getInstance().getSettings();

    Diligent::CopyTextureAttribs CopyAttribs(m_pRTColorOutput, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                             m_pStagingTexture,
                                             Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    contextData.immediateContext->CopyTexture(CopyAttribs);

    contextData.immediateContext->WaitForIdle();
    Diligent::MappedTextureSubresource MappedData;
    contextData.immediateContext->MapTextureSubresource(
        m_pStagingTexture, 0, 0, Diligent::MAP_READ, Diligent::MAP_FLAG_DO_NOT_WAIT, nullptr, MappedData
        );
    auto pData = static_cast<uint8_t*>(MappedData.pData);
    auto rowPitch = MappedData.Stride;

    uint32_t pixelOffset = (settings.height - position.y) * (rowPitch) + (position.x * 4);
    uint8_t r = pData[pixelOffset + 0];
    uint8_t g = pData[pixelOffset + 1];
    uint8_t b = pData[pixelOffset + 2];
    uint8_t a = pData[pixelOffset + 3];

    // 5. Unmap the texture
    contextData.immediateContext->UnmapTextureSubresource(m_pStagingTexture, 0, 0);

    PrismaFunc::getInstance().bindMainRenderTarget();

    uint32_t encodedUUID = (r << 16) | (g << 8) | b - 1;
    if (encodedUUID >= 0) {
        if (a < 128) {
            if (encodedUUID < GlobalData::getInstance().currentGlobalScene()->meshes.size()) {
                return GlobalData::getInstance().currentGlobalScene()->meshes[encodedUUID];
            }
        } else {
            if (encodedUUID < GlobalData::getInstance().currentGlobalScene()->animateMeshes.size()) {
                return GlobalData::getInstance().currentGlobalScene()->animateMeshes[encodedUUID];
            }
        }
    }

    return nullptr;
}

void Prisma::GUI::PixelCapture::createDrawPipeline() {
    auto settings = SettingsLoader::getInstance().getSettings();
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "ImGui Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
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

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(
        nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui VS";
        ShaderCI.FilePath = "../../../GUI/Shaders/PixelCapture/vertex.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui PS";
        ShaderCI.FilePath = "../../../GUI/Shaders/PixelCapture/fragment.glsl";
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
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::ShaderResourceVariableDesc Vars[] =
    {
        {Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(),
         Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    Diligent::SamplerDesc SamLinearClampDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP
    };
    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0;
    RTColorDesc.ClearValue.Color[1] = 0;
    RTColorDesc.ClearValue.Color[2] = 0;
    RTColorDesc.ClearValue.Color[3] = 1;
    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_pRTColor);


    // Create window-size depth buffer
    Diligent::TextureDesc RTDepthDesc = RTColorDesc;
    RTDepthDesc.Name = "Offscreen depth buffer PX capture";
    RTDepthDesc.Format = PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    RTDepthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;
    // Define optimal clear value
    RTDepthDesc.ClearValue.Format = RTDepthDesc.Format;
    RTDepthDesc.ClearValue.DepthStencil.Depth = 1;
    RTDepthDesc.ClearValue.DepthStencil.Stencil = 0;
    contextData.device->CreateTexture(RTDepthDesc, nullptr, &m_pRTDepth);

    // Store the depth-stencil view
    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());

    m_pso->CreateShaderResourceBinding(&m_srb, true);

    MeshIndirect::getInstance().addResizeHandler({"PixelCaptureMesh handler" ,[&](auto buffer, auto& material)
        {
            m_srb.Release();
            m_pso->CreateShaderResourceBinding(&m_srb, true);
            m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
        }});
    GlobalData::getInstance().addGlobalTexture({ m_pRTColor ,"PixelCapture",{contextData.swapChain->GetDesc().Width,contextData.swapChain->GetDesc().Height} });
}

void Prisma::GUI::PixelCapture::createDrawAnimationPipeline()
{
    auto settings = SettingsLoader::getInstance().getSettings();
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "ImGui Render Animation";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
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

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(
        nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui Animation VS";
        ShaderCI.FilePath = "../../../GUI/Shaders/PixelCapture/vertex_animation.glsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui Animation PS";
        ShaderCI.FilePath = "../../../GUI/Shaders/PixelCapture/fragment_animation.glsl";
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
    PSOCreateInfo.pPS = pPS;

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    Diligent::ShaderResourceVariableDesc Vars[] = {{Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str(),
                                                    Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}};
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    Diligent::SamplerDesc SamLinearClampDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP
    };
    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_psoAnimation);

    // Store the depth-stencil view
    m_psoAnimation->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(MeshHandler::getInstance().viewProjection());
    m_psoAnimation->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::CONSTANT_ANIMATION.c_str())->Set(AnimationHandler::getInstance().animation()->GetDefaultView(
                                Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    m_psoAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);

    if (MeshIndirect::getInstance().modelBufferAnimation()) {
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBufferAnimation()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
    }

    MeshIndirect::getInstance().addResizeHandler({"PixelCaptureAnimation handler" ,[&](auto buffer, auto& material)
        {
            m_srbAnimation.Release();
            m_psoAnimation->CreateShaderResourceBinding(&m_srbAnimation, true);
            m_srbAnimation->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, ShaderNames::MUTABLE_MODELS.c_str())->Set(MeshIndirect::getInstance().modelBufferAnimation()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
        }});
}

void Prisma::GUI::PixelCapture::createScalePipeline()
{
    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "ImGui Scale Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA8_UNORM;
    // Set depth buffer format which is the format of the swap chain's back buffer
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
    // Enable depth testing
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;

    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // Pack matrices in row-major order
    ShaderCI.CompileFlags = Diligent::SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;

    // In this tutorial, we will load shaders from file. To be able to do that,
    // we need to create a shader source stream factory
    Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> pShaderSourceFactory;
    PrismaFunc::getInstance().contextData().engineFactory->CreateDefaultShaderSourceStreamFactory(
        nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/SceneRender/vertex.hlsl";
        contextData.device->CreateShader(ShaderCI, &pVS);
        // Create dynamic uniform buffer that will store our transformation matrix
        // Dynamic buffers can be frequently updated by the CPU
        Diligent::BufferDesc CBDesc;
        CBDesc.Name = "VS";
        CBDesc.Size = sizeof(glm::mat4);
        CBDesc.Usage = Diligent::USAGE_DYNAMIC;
        CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        contextData.device->CreateBuffer(CBDesc, nullptr, &m_mvpVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "ImGui PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/SceneRender/fragment_capture.hlsl";
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

    Diligent::ShaderResourceVariableDesc Vars[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    Diligent::SamplerDesc SamLinearClampDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP
    };
    Diligent::ImmutableSamplerDesc ImtblSamplers[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &m_scalePso);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    RTColorDesc.Width = contextData.swapChain->GetDesc().Width;
    RTColorDesc.Height = contextData.swapChain->GetDesc().Height;
    RTColorDesc.MipLevels = 1;
    RTColorDesc.Format = PSOCreateInfo.GraphicsPipeline.RTVFormats[0];
    // The render target can be bound as a shader resource and as a render target
    RTColorDesc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_SHADER_RESOURCE;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.ClearValue.Color[0] = 0;
    RTColorDesc.ClearValue.Color[1] = 0;
    RTColorDesc.ClearValue.Color[2] = 0;
    RTColorDesc.ClearValue.Color[3] = 1;

    Diligent::TextureDesc StagingTexDesc = RTColorDesc;
    StagingTexDesc.Usage = Diligent::USAGE_STAGING;
    StagingTexDesc.BindFlags = Diligent::BIND_NONE;
    StagingTexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;

    contextData.device->CreateTexture(StagingTexDesc, nullptr, &m_pStagingTexture);

    contextData.device->CreateTexture(RTColorDesc, nullptr, &m_pRTColorOutput);

    m_scalePso->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(m_mvpVS);
    m_scalePso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(
        m_pRTColor->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    GlobalData::getInstance().addGlobalTexture({m_pRTColorOutput, "PixelCapture Resize",
                                                {contextData.swapChain->GetDesc().Width,
                                                 contextData.swapChain->GetDesc().Height}});

    m_scalePso->CreateShaderResourceBinding(&m_scaleSrb, true);
}

void Prisma::GUI::PixelCapture::drawModel(const glm::mat4& model) {
    auto& contextData = PrismaFunc::getInstance().contextData();

    contextData.immediateContext->SetPipelineState(m_scalePso);

    auto rt = m_pRTColorOutput->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);

    contextData.immediateContext->SetRenderTargets(1, &rt, nullptr,
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->ClearRenderTarget(rt, value_ptr(m_clearColor),
                                                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    auto quadBuffer = PrismaRender::getInstance().quadBuffer();

    // Bind vertex and index buffers
    constexpr Diligent::Uint64 offset = 0;
    Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
    contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset,
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                                   Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    contextData.immediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0,
                                                 Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    {
        // Map the buffer and write current world-view-projection matrix
        Diligent::MapHelper<glm::mat4> CBConstants(contextData.immediateContext, m_mvpVS,
                                                   Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
        *CBConstants = model;
    }

    // Set texture SRV in the SRB
    contextData.immediateContext->CommitShaderResources(
        m_scaleSrb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Diligent::DrawIndexedAttribs DrawAttrs; // This is an indexed draw call
    DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
    DrawAttrs.NumIndices = quadBuffer.iBufferSize;
    // Verify the state of vertex and index buffers
    DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
    contextData.immediateContext->DrawIndexed(DrawAttrs);
    PrismaFunc::getInstance().bindMainRenderTarget();
}