#include "SceneObjects/Sprite.h"
#include "Helpers/PrismaRender.h"
#include <glm/gtx/string_cast.hpp>

#include "GlobalData/GlobalShaderNames.h"
#include "GlobalData/PrismaFunc.h"
#include "Handlers/MeshHandler.h"
#include "Pipelines/PipelineHandler.h"

Prisma::Sprite::Sprite()
{
	//m_spriteShader = std::make_shared<Shader>("../../../Engine/Shaders/SpritePipeline/vertex.glsl",
	//                                          "../../../Engine/Shaders/SpritePipeline/fragment.glsl", nullptr);
	//m_spriteShader->use();
	//m_spritePos = m_spriteShader->getUniformPosition("sprite");
	//m_modelPos = m_spriteShader->getUniformPosition("model");
	//m_sizePos = m_spriteShader->getUniformPosition("billboardSize");
	//m_ssbo = std::make_shared<SSBO>(12);
	//m_ssboTextures = std::make_shared<SSBO>(13);
	//m_ssboIds = std::make_shared<SSBO>(14);

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSOCreateInfo.PSODesc.Name = "Sprite Pipeline";

    // This is a graphics pipeline
    PSOCreateInfo.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;

    // clang-format off
    // This tutorial will render to a single render target
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = Prisma::PrismaFunc::getInstance().renderFormat().RenderFormat;
    // Set depth buffer format which is the format of the swap chain's back buffer
    PSOCreateInfo.GraphicsPipeline.DSVFormat = Prisma::PrismaFunc::getInstance().renderFormat().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FrontCounterClockwise = true;
    // Cull back faces
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_BACK;
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
        ShaderCI.Desc.Name = "Sprite VS";
        ShaderCI.FilePath = "../../../Engine/Shaders/SpritePipeline/vertex.glsl";
        contextData.m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Sprite PS";
        ShaderCI.FilePath = "../../../Engine/Shaders/SpritePipeline/fragment.glsl";
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


	Diligent::PipelineResourceDesc Resources[] =
    {
        {Diligent::SHADER_TYPE_VERTEX, "SpritesData", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str(), 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        {Diligent::SHADER_TYPE_PIXEL, "SpriteIds", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        {Diligent::SHADER_TYPE_PIXEL,"spriteTextures",Define::MAX_SPRITES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
        {Diligent::SHADER_TYPE_PIXEL,"textureClamp_sampler",1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
    };

	Diligent::PipelineResourceSignatureDesc ResourceSignDesc;
    ResourceSignDesc.NumResources = _countof(Resources);
    ResourceSignDesc.Resources = Resources;


    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
	Diligent::SamplerDesc SamLinearClampDesc
    {
	    Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR, Diligent::FILTER_TYPE_LINEAR,
	    Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP, Diligent::TEXTURE_ADDRESS_CLAMP
    };

	Diligent::RefCntAutoPtr<Diligent::ISampler> samplerClamp;

    contextData.m_pDevice->CreatePipelineResourceSignature(ResourceSignDesc, &m_pResourceSignature);

	Diligent::IPipelineResourceSignature* ppSignatures[]{ m_pResourceSignature };

    PSOCreateInfo.ppResourceSignatures = ppSignatures;
    PSOCreateInfo.ResourceSignaturesCount = _countof(ppSignatures);

    contextData.m_pDevice->CreatePipelineState(PSOCreateInfo, &m_pso);
    contextData.m_pDevice->CreateSampler(SamLinearClampDesc, &samplerClamp);
    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "textureClamp_sampler")->Set(samplerClamp);

    m_pResourceSignature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, Prisma::ShaderNames::CONSTANT_VIEW_PROJECTION.c_str())->Set(Prisma::MeshHandler::getInstance().viewProjection());

    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

    Diligent::BufferDesc ModelDesc;
    ModelDesc.Name = "Sprite Models Buffer";
    ModelDesc.Usage = Diligent::USAGE_DEFAULT;
    ModelDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    ModelDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    ModelDesc.ElementByteStride = sizeof(glm::mat4);
    ModelDesc.Size = sizeof(glm::mat4);
    contextData.m_pDevice->CreateBuffer(ModelDesc, nullptr, &m_models);

    Diligent::BufferDesc SpriteDesc;
    SpriteDesc.Name = "Sprite Identifier Buffer";
    SpriteDesc.Usage = Diligent::USAGE_DEFAULT;
    SpriteDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    SpriteDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    SpriteDesc.ElementByteStride = sizeof(glm::ivec4);
    SpriteDesc.Size = sizeof(glm::ivec4);
    contextData.m_pDevice->CreateBuffer(SpriteDesc, nullptr, &m_spriteIds);

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "SpritesData")->Set(m_models->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SpriteIds")->Set(m_spriteIds->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    //m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SpriteTextures")->SetArray(nullptr, 0, 0, Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);

}

void Prisma::Sprite::loadSprites(std::vector<std::shared_ptr<Texture>> textures)
{
	m_sprites.clear();

	for (auto sprite : textures)
	{
		m_sprites.push_back(sprite->texture()->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));
	}

    m_srb.Release();
    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "SpritesData")->Set(m_models->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SpriteIds")->Set(m_spriteIds->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "spriteTextures")->SetArray(m_sprites.data(), 0, m_sprites.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
	//m_ssboTextures->resize(sizeof(SpriteData) * m_spritesData.size());
	//m_ssboTextures->modifyData(0, sizeof(SpriteData) * m_spritesData.size(), m_spritesData.data());
}

void Prisma::Sprite::numSprites(unsigned int numSprites)
{
	m_numSprites = numSprites;
	//m_ssbo->resize(sizeof(glm::mat4) * m_numSprites);
	//m_ssboIds->resize(sizeof(glm::ivec4) * m_numSprites);
	std::vector<glm::mat4> spriteModels;
    std::vector<glm::ivec4> spriteIndices;
	spriteModels.resize(m_numSprites);
    spriteIndices.resize(m_numSprites);
	glm::mat4 defaultData(1.0f);
    glm::ivec4 defaultIndices(0.0f);
	for (int i = 0; i < m_numSprites; i++)
	{
		spriteModels[i] = defaultData;
        spriteIndices[i] = defaultIndices;
	}
	//m_ssbo->modifyData(0, sizeof(glm::mat4) * m_numSprites, spriteModels.data());
    m_models.Release();
    m_spriteIds.Release();

    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    Diligent::BufferDesc ModelDesc;
    ModelDesc.Name = "Sprite Models Buffer";
    ModelDesc.Usage = Diligent::USAGE_DEFAULT;
    ModelDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    ModelDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    ModelDesc.ElementByteStride = sizeof(glm::mat4);
    ModelDesc.Size = sizeof(glm::mat4)*m_numSprites;
	Diligent::BufferData modelData;
    modelData.DataSize = ModelDesc.Size;
    modelData.pData = spriteModels.data();

    contextData.m_pDevice->CreateBuffer(ModelDesc, &modelData, &m_models);

    Diligent::BufferDesc SpriteDesc;
    SpriteDesc.Name = "Sprite Identifier Buffer";
    SpriteDesc.Usage = Diligent::USAGE_DEFAULT;
    SpriteDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    SpriteDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
    SpriteDesc.ElementByteStride = sizeof(glm::ivec4);
    SpriteDesc.Size = sizeof(glm::ivec4)*m_numSprites;
    Diligent::BufferData indicesData;
    indicesData.DataSize = SpriteDesc.Size;
    indicesData.pData = spriteIndices.data();

    contextData.m_pDevice->CreateBuffer(SpriteDesc, &indicesData, &m_spriteIds);

    m_srb.Release();
    m_pResourceSignature->CreateShaderResourceBinding(&m_srb, true);

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "SpritesData")->Set(m_models->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "SpriteIds")->Set(m_spriteIds->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));

    m_srb->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "spriteTextures")->SetArray(m_sprites.data(), 0, m_sprites.size(), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
}

void Prisma::Sprite::size(glm::vec2 size)
{
	m_size = size;
}

void Prisma::Sprite::render()
{
	if (visible())
	{
		//m_spriteShader->use();
		//m_spriteShader->setMat4(m_modelPos, finalMatrix());
		//m_spriteShader->setVec2(m_sizePos, m_size);
		//// Enable additive blending
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE);
		//glDepthMask(GL_FALSE);
		//// Render the sprites
		////PrismaRender::getInstance().renderQuad(m_numSprites);
		//glDepthMask(GL_TRUE);

		//// Deactivate blending and restore OpenGL state
		//glDisable(GL_BLEND);
        auto& contextData = Prisma::PrismaFunc::getInstance().contextData();
        // Set the pipeline state
        contextData.m_pImmediateContext->SetPipelineState(m_pso);

        auto quadBuffer = Prisma::PrismaRender::getInstance().quadBuffer();

        // Bind vertex and index buffers
        const Diligent::Uint64 offset = 0;
        Diligent::IBuffer* pBuffs[] = { quadBuffer.vBuffer };
        contextData.m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        contextData.m_pImmediateContext->SetIndexBuffer(quadBuffer.iBuffer, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        contextData.m_pImmediateContext->CommitShaderResources(m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        Diligent::DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
        DrawAttrs.IndexType = Diligent::VT_UINT32; // Index type
        DrawAttrs.NumIndices = quadBuffer.iBufferSize;
        // Verify the state of vertex and index buffers
        DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        DrawAttrs.NumInstances = m_numSprites;
        contextData.m_pImmediateContext->DrawIndexed(DrawAttrs);

	}
}
