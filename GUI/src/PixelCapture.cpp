#include "../include/PixelCapture.h"
#include "../../Engine/include/Helpers/SettingsLoader.h"
#include "../../Engine/include/Helpers/PrismaRender.h"
#include "../../Engine/include/SceneData/MeshIndirect.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../../Engine/include/Handlers/MeshHandler.h"

static std::shared_ptr<Prisma::Shader> shader = nullptr;
static std::shared_ptr<Prisma::Shader> shaderAnimation = nullptr;
static std::shared_ptr<Prisma::Shader> shaderData = nullptr;

Prisma::PixelCapture::PixelCapture()
{
	if (!shader)
	{
		auto settings = SettingsLoader::getInstance().getSettings();

		FBO::FBOData fboData;
		fboData.enableDepth = true;
		fboData.height = settings.height;
		fboData.width = settings.width;
		fboData.name = "CAPTURES";

		m_fbo = std::make_shared<FBO>(fboData);
		m_fboData = std::make_shared<FBO>(fboData);

		shader = std::make_shared<Shader>("../../../GUI/Shaders/PixelCapture/vertex.glsl",
		                                  "../../../GUI/Shaders/PixelCapture/fragment.glsl");

		shaderData = std::make_shared<Shader>("../../../GUI/Shaders/Output/vertex.glsl",
		                                      "../../../GUI/Shaders/Output/fragment.glsl");

		shaderAnimation = std::make_shared<Shader>("../../../GUI/Shaders/PixelCapture/vertex_animation.glsl",
		                                           "../../../GUI/Shaders/PixelCapture/fragment_animation.glsl");
		shaderData->use();
		m_modelLocation = shaderData->getUniformPosition("model");
		m_textureLocation = shaderData->getUniformPosition("screenTexture");
	}
}

std::shared_ptr<Prisma::Mesh> Prisma::PixelCapture::capture(glm::vec2 position, const glm::mat4& model)
{
	m_fbo->bind();
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
	return nullptr;
}
