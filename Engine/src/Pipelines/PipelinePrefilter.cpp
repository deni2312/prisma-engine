#include "../../include/Pipelines/PipelinePrefilter.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Pipelines/PipelineDIffuseIrradiance.h"
#include "../../include/Helpers/PrismaRender.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../../include/Helpers/SettingsLoader.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"


Prisma::PipelinePrefilter::PipelinePrefilter()
{
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Skybox Render";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
    // Enable depth testing
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

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
    Prisma::PrismaFunc::getInstance().contextData().m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Skybox VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/PrefilterPipeline/vertex.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Skybox PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/PrefilterPipeline/fragment.hlsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pPS);
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
        {Diligent::SHADER_TYPE_PIXEL, "environmentMap", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    Diligent::SamplerDesc SamLinearClampDesc
    {
        Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
        Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP
    };

    Diligent::ImmutableSamplerDesc ImtblSamplers[] =
    {
        {Diligent::SHADER_TYPE_PIXEL, "environmentMap", SamLinearClampDesc}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
    contextData.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pso);

    Diligent::BufferDesc CBDesc;
    CBDesc.Name = "IBLData";
    CBDesc.Size = sizeof(Prisma::PipelineSkybox::IBLViewProjection);
    CBDesc.Usage = Diligent::USAGE_DYNAMIC;
    CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    contextData.m_pDevice->CreateBuffer(CBDesc, nullptr, &m_iblData);

    Diligent::BufferDesc CBDescRL;
    CBDescRL.Name = "IBLData";
    CBDescRL.Size = sizeof(RoughnessResolution);
    CBDescRL.Usage = Diligent::USAGE_DYNAMIC;
    CBDescRL.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    CBDescRL.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

    contextData.m_pDevice->CreateBuffer(CBDescRL, nullptr, &m_iblResolution);

    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "IBLData")->Set(m_iblData);

    m_pso->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "RoughnessResolution")->Set(m_iblResolution);

    m_pso->CreateShaderResourceBinding(&m_srb, true);

    Diligent::TextureDesc RTColorDesc;
    RTColorDesc.Name = "Offscreen render target";
    RTColorDesc.Type = Diligent::RESOURCE_DIM_TEX_CUBE;  // Set cubemap type
    RTColorDesc.Width = m_dimensions.x;
    RTColorDesc.Height = m_dimensions.y;
    RTColorDesc.MipLevels = 8;
    RTColorDesc.Format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
    // Specify 6 faces for cubemap
    RTColorDesc.ArraySize = 6;
    // Allow it to be used as both a shader resource and a render target
    RTColorDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    // Define optimal clear value
    RTColorDesc.ClearValue.Format = RTColorDesc.Format;
    RTColorDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_GENERATE_MIPS;
    // Create the cubemap texture
    contextData.m_pDevice->CreateTexture(RTColorDesc, nullptr, &m_pMSColorRTV);



}

void Prisma::PipelinePrefilter::texture(Diligent::RefCntAutoPtr<Diligent::ITexture> texture)
{
    m_srb.Release();
    m_pso->CreateShaderResourceBinding(&m_srb, true);
    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "environmentMap")->Set(texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    unsigned int maxMipLevels = 8;

    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        //unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        //unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));

        float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);

        Diligent::TextureViewDesc RTVDesc;
        RTVDesc.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
        RTVDesc.TextureDim = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
        RTVDesc.MostDetailedMip = mip;
        RTVDesc.NumMipLevels = 1;

        for (unsigned int i = 0; i < 6; ++i) {
            Diligent::MapHelper<Prisma::PipelineSkybox::IBLViewProjection> viewProjection(contextData.m_pImmediateContext, m_iblData, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
            viewProjection->view = m_iblTransform.captureViews[i];
            viewProjection->projection = m_iblTransform.captureProjection;

            Diligent::MapHelper<RoughnessResolution> roughnessResolution(contextData.m_pImmediateContext, m_iblResolution, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
            roughnessResolution->roughness = roughness;
            roughnessResolution->resolution = 4096;
            RTVDesc.FirstArraySlice = i;  // Select the specific face
            RTVDesc.NumArraySlices = 1;

            Diligent::RefCntAutoPtr<Diligent::ITextureView> pRTV;

            m_pMSColorRTV->CreateView(RTVDesc, &pRTV);

            Diligent::TextureViewDesc RTVDesc{ "Skybox", Diligent::TEXTURE_VIEW_RENDER_TARGET, Diligent::RESOURCE_DIM_TEX_2D_ARRAY };

            Diligent::ITextureView* ppRTVs[] = { pRTV };

            contextData.m_pImmediateContext->SetRenderTargets(1, ppRTVs, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            contextData.m_pImmediateContext->ClearRenderTarget(pRTV, glm::value_ptr(glm::vec4(Prisma::Define::CLEAR_COLOR)), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            contextData.m_pImmediateContext->SetPipelineState(m_pso);

            auto cubeBuffer = Prisma::PrismaRender::getInstance().cubeBuffer();

            // Bind vertex and index buffers
            const Diligent::Uint64 offset = 0;
            Diligent::IBuffer* pBuffs[] = { cubeBuffer.vBuffer };
            contextData.m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
            contextData.m_pImmediateContext->SetIndexBuffer(cubeBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            // Set texture SRV in the SRB
            contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
            DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
            DrawAttrs.NumIndices = cubeBuffer.iBufferSize;
            // Verify the state of vertex and index buffers
            DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
            contextData.m_pImmediateContext->DrawIndexed(DrawAttrs);
        }
    }
    
    Prisma::PrismaFunc::getInstance().bindMainRenderTarget();
	//if (m_id)
	//{
	//	glMakeTextureHandleNonResidentARB(m_id);
	//	glDeleteTextures(1, &m_prefilterMap);
	//}

	//// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	//// --------------------------------------------------------------------------------
	//glGenTextures(1, &m_prefilterMap);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, m_prefilterMap);
	//for (unsigned int i = 0; i < 6; ++i)
	//{
	//	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	//}
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//// be sure to set minification filter to mip_linear 
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//// generate mipmaps for the cubemap so Prisma automatically allocates the required memory.
	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	//// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	//// ----------------------------------------------------------------------------------------------------
	////m_shader->use();
	////m_shader->setInt64(m_shader->getUniformPosition("environmentMap"), texture.id());
	////m_shader->setInt(m_shader->getUniformPosition("resolution"), texture.data().width);
	////m_shader->setMat4(m_shader->getUniformPosition("projection"), PrismaRender::getInstance().data().captureProjection);

	//glBindFramebuffer(GL_FRAMEBUFFER, PrismaRender::getInstance().data().fbo);
	//glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.

	//unsigned int maxMipLevels = 8;
	//for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	//{
	//	// reisze framebuffer according to mip-level size.
	//	unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
	//	unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
	//	glBindRenderbuffer(GL_RENDERBUFFER, PrismaRender::getInstance().data().rbo);
	//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
	//	glViewport(0, 0, mipWidth, mipHeight);

	//	float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
	//	m_shader->setFloat(m_shader->getUniformPosition("roughness"), roughness);
	//	for (unsigned int i = 0; i < 6; ++i)
	//	{
	//		m_shader->setMat4(m_shader->getUniformPosition("view"), PrismaRender::getInstance().data().captureViews[i]);
	//		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
	//			m_prefilterMap, mip);

	//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//		//PrismaRender::getInstance().renderCube();
	//	}
	//}
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, Prisma::SettingsLoader().getInstance().getSettings().width,
	//           Prisma::SettingsLoader().getInstance().getSettings().height);
	//// don't forget to configure the viewport to the capture dimensions.
	//m_id = glGetTextureHandleARB(m_prefilterMap);
	//glMakeTextureHandleResidentARB(m_id);
}

Diligent::RefCntAutoPtr<Diligent::ITexture> Prisma::PipelinePrefilter::prefilterTexture()
{
    return m_pMSColorRTV;
}
