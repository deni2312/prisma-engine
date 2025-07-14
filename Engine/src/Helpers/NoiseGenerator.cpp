#include "Helpers/NoiseGenerator.h"

#include "GlobalData/PrismaFunc.h"
#include "Helpers/PrismaRender.h"
#include "Pipelines/PipelineHandler.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>


Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::NoiseGenerator::generate(const std::string& vertex, const std::string& fragment, glm::vec2 resolution, const std::string& name, NoiseType type) {
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> srb;

    auto& contextData = PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Noise Generator";

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

    std::string widthStr = std::to_string(static_cast<int>(resolution.x));
    std::string heightStr = std::to_string(static_cast<int>(resolution.y));

    Diligent::ShaderMacro Macros[] = {
        {"WIDTH", widthStr.c_str()}, {"HEIGHT", heightStr.c_str()}
    };
    ShaderCI.Macros = {Macros, _countof(Macros)};
    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
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
        ShaderCI.Desc.Name = "Noise VS";
        ShaderCI.FilePath = vertex.c_str();
        contextData.device->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Noise PS";
        ShaderCI.FilePath = fragment.c_str();
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
    contextData.device->CreateGraphicsPipelineState(PSOCreateInfo, &pso);

    
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Noise Texture";

    switch (type) {
        case NoiseType::TEXTURE_2D: {
            RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;  // Set cubemap type
        } break;
        case NoiseType::TEXTURE_3D: {
            RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;  // Set cubemap type
            RTColorDesc.ArraySize = 6;


            Diligent::BufferDesc CBDesc;
            CBDesc.Name = "Constants";
            CBDesc.Size = sizeof(PipelineSkybox::IBLViewProjection);
            CBDesc.Usage = Diligent::USAGE_DYNAMIC;
            CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
            CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

            contextData.device->CreateBuffer(CBDesc, nullptr, &buffer);

            pso->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(buffer);

        } break;
    }
    RTColorDesc.Width = resolution.x;
    RTColorDesc.Height = resolution.y;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    // Specify 6 faces for cubemap
    // Allow it to be used as both a shader resource and a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    // Create the cubemap texture
    contextData.device->CreateTexture(RTColorDesc, nullptr, &texture);


    pso->CreateShaderResourceBinding(&srb, true);






    switch (type) {
        case NoiseType::TEXTURE_2D: {
            auto color = texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
            contextData.immediateContext->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            contextData.immediateContext->SetPipelineState(pso);
            auto quadBuffer = PrismaRender::getInstance().quadBuffer();

            // Bind vertex and index buffers
            constexpr Diligent::Uint64 offset = 0;
            Diligent::IBuffer* pBuffs[] = {quadBuffer.vBuffer};
            contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            contextData.immediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            // Set texture SRV in the SRB
            contextData.immediateContext->CommitShaderResources(srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
            DrawAttrs.IndexType = Diligent::VT_UINT32;  // Index type
            DrawAttrs.NumIndices = quadBuffer.iBufferSize;
            // Verify the state of vertex and index buffers
            DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
            contextData.immediateContext->DrawIndexed(DrawAttrs);

            GlobalData::getInstance().addGlobalTexture({texture, name});
        } break;
        case NoiseType::TEXTURE_3D: {
            auto color = texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
            Diligent::RefCntAutoPtr<Diligent::ITextureView> pRTColor[6];
            const PipelineSkybox::IBLData transform;
                // Create render target views for each face
            for (int i = 0; i < 6; ++i) {
                Diligent::TextureViewDesc RTVDesc;
                RTVDesc.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
                RTVDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
                RTVDesc.MostDetailedMip = 0;
                RTVDesc.NumMipLevels = 1;
                RTVDesc.FirstArraySlice = i;  // Select the specific face
                RTVDesc.NumArraySlices = 1;

                texture->CreateView(RTVDesc, &pRTColor[i]);

                Diligent::MapHelper<PipelineSkybox::IBLViewProjection> viewProjection(contextData.immediateContext, buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
                viewProjection->view = transform.captureViews[i];
                viewProjection->projection = transform.captureProjection;



                contextData.immediateContext->SetRenderTargets(1, &pRTColor[i], nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                contextData.immediateContext->ClearRenderTarget(pRTColor[i], value_ptr(Define::CLEAR_COLOR), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                contextData.immediateContext->SetPipelineState(pso);

                auto cubeBuffer = PrismaRender::getInstance().cubeBuffer();

                // Bind vertex and index buffers
                constexpr Diligent::Uint64 offset = 0;
                Diligent::IBuffer* pBuffs[] = {cubeBuffer.vBuffer};
                contextData.immediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
                contextData.immediateContext->SetIndexBuffer(cubeBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                // Set texture SRV in the SRB
                contextData.immediateContext->CommitShaderResources(srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
                DrawAttrs.IndexType = Diligent::VT_UINT32;  // Index type
                DrawAttrs.NumIndices = cubeBuffer.iBufferSize;
                // Verify the state of vertex and index buffers
                DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
                contextData.immediateContext->DrawIndexed(DrawAttrs);
                //GlobalData::getInstance().addGlobalTexture({texture, name});
            }
        } break;
    }


    return texture;
}