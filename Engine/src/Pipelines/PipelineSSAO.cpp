#include "../../include/Pipelines/PipelineSSAO.h"
#include "../../include/Helpers/SettingsLoader.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Helpers/GarbageCollector.h"
#include "../../include/Helpers/PrismaRender.h"

#include <random>
#include <glm/gtx/string_cast.hpp>

Prisma::PipelineSSAO::PipelineSSAO() {
	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/SSAOPipeline/vertex.glsl",
		"../../../Engine/Shaders/SSAOPipeline/fragment.glsl");

    m_shader->use();

    m_depthPos = m_shader->getUniformPosition("textureNormal");
    m_positionPos = m_shader->getUniformPosition("texturePosition");
    m_noisePos = m_shader->getUniformPosition("textureNoise");
    m_noiseScalePos = m_shader->getUniformPosition("noiseScale");
    m_shaderBlur = std::make_shared<Shader>("../../../Engine/Shaders/SSAOBlurPipeline/vertex.glsl",
        "../../../Engine/Shaders/SSAOBlurPipeline/fragment.glsl");
    m_shaderBlur->use();
    m_ssaoPos = m_shaderBlur->getUniformPosition("ssaoInput");

	auto settings = SettingsLoader::getInstance().getSettings();

	FBO::FBOData fboData;
	fboData.width = settings.width;
	fboData.height = settings.height;
	fboData.enableDepth = true;
	fboData.internalFormat = GL_RED;
	fboData.internalType = GL_FLOAT;
	fboData.name = "SSAO";
	m_fbo = std::make_shared<FBO>(fboData);

    FBO::FBOData fboDataBlur;
    fboDataBlur.width = settings.width;
    fboDataBlur.height = settings.height;
    fboDataBlur.enableDepth = true;
    fboDataBlur.internalFormat = GL_RED;
    fboDataBlur.internalType = GL_FLOAT;
    fboDataBlur.name = "SSAO_BLUR";
    m_fboBlur = std::make_shared<FBO>(fboDataBlur);

    m_scale = glm::vec2(settings.width/4, settings.height/4);

    // generate sample kernel
 // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec4> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(glm::vec4(sample,1.0f));
    }

    // generate noise texture
    // ----------------------
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture; 
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    m_noise = glGetTextureHandleARB(noiseTexture);
    glMakeTextureHandleResidentARB(m_noise);
    Prisma::GlobalData::getInstance().addGlobalTexture({ noiseTexture, "SSAONoise",{4,4}});
    Prisma::GarbageCollector::getInstance().addTexture({ noiseTexture, m_noise });

    m_ubo = std::make_shared<Prisma::Ubo>(sizeof(glm::vec4)* ssaoKernel.size(), 4);
    m_ubo->modifyData(0, sizeof(glm::vec4) * ssaoKernel.size(), ssaoKernel.data());
    
}

void Prisma::PipelineSSAO::update(uint64_t depth, uint64_t position) {
    m_fbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    m_shader->use();
    m_shader->setInt64(m_positionPos, position);
    m_shader->setInt64(m_depthPos, depth);
    m_shader->setInt64(m_noisePos, m_noise);
    m_shader->setVec2(m_noiseScalePos, m_scale);

    //Prisma::PrismaRender::getInstance().renderQuad();
    m_fbo->unbind();

    m_fboBlur->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    m_shaderBlur->use();
    m_shaderBlur->setInt64(m_ssaoPos,m_fbo->texture());
    //Prisma::PrismaRender::getInstance().renderQuad();
    m_fboBlur->unbind();

}

std::shared_ptr<Prisma::FBO> Prisma::PipelineSSAO::texture() {
    return m_fbo;
}

float Prisma::PipelineSSAO::ourLerp(float a, float b, float f)
{
	return a + f * (b - a);
}
