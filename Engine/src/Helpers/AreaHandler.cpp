#include "../../include/Helpers/AreaHandler.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../../include/Helpers/AreaData.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/Helpers/GarbageCollector.h"


Prisma::AreaHandler::AreaHandler() {
	unsigned int textureLut = 0;
	glGenTextures(1, &textureLut);
	glBindTexture(GL_TEXTURE_2D, textureLut);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
		0, GL_RGBA, GL_FLOAT, LTC1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned int textureMt = 0;
	glGenTextures(1, &textureMt);
	glBindTexture(GL_TEXTURE_2D, textureMt);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64,
		0, GL_RGBA, GL_FLOAT, LTC2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_idLut = glGetTextureHandleARB(textureLut);
	glMakeTextureHandleResidentARB(m_idLut);

	m_idM = glGetTextureHandleARB(textureMt);
	glMakeTextureHandleResidentARB(m_idM);

	Prisma::GlobalData::getInstance().addGlobalTexture({ textureLut, "TextureLut"});
	GarbageCollector::getInstance().addTexture({ textureLut, m_idLut });

	Prisma::GlobalData::getInstance().addGlobalTexture({ textureMt, "TextureM" });
	GarbageCollector::getInstance().addTexture({ textureMt, m_idM });
}

uint64_t Prisma::AreaHandler::idLut() {
	return m_idLut;
}

uint64_t Prisma::AreaHandler::idM() {
	return m_idM;
}
