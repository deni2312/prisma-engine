#include "Pipelines/PipelineSoftwareRT.h"

#include "GlobalData/PrismaFunc.h"

Prisma::PipelineSoftwareRT::PipelineSoftwareRT(unsigned int width, unsigned int height) {
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
        ShaderCI.Desc.Name = "Software RT CS";
        ShaderCI.FilePath = "../../../Engine/Shaders/SoftwareRTPipeline/compute.glsl";
        contextData.device->CreateShader(ShaderCI, &pResetParticleListsCS);
    }

    Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
    Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

    // This is a compute pipeline
    PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;

    PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
    // clang-format off
    /*	Diligent::ShaderResourceVariableDesc Vars[] =
            {
    
            };
        // clang-format on
        PSODesc.ResourceLayout.Variables = Vars;
        PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        */
    PSODesc.Name = "Software RT";
    PSOCreateInfo.pCS = pResetParticleListsCS;
    contextData.device->CreateComputePipelineState(PSOCreateInfo, &m_pso);

    m_pso->CreateShaderResourceBinding(&m_srb, true);
}

void Prisma::PipelineSoftwareRT::render() {
    auto& contextData = PrismaFunc::getInstance().contextData();

    Diligent::DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = 1;
    DispatAttribs.ThreadGroupCountY = 1;
    DispatAttribs.ThreadGroupCountZ = 1;
    contextData.immediateContext->SetPipelineState(m_pso);
    contextData.immediateContext->CommitShaderResources(
        m_srb, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    contextData.immediateContext->DispatchCompute(DispatAttribs);
}