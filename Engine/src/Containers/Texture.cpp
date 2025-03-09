#include "../../include/Containers/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../include/Helpers/stb_image.h"
#include "GL/glew.h"
#include "../../include/Helpers/GarbageCollector.h"
#include <iostream>
#include <tuple>
#include "../../include/GlobalData/GlobalData.h"

#include "../../include/Helpers/Logger.h"
#include "TextureLoader/interface/TextureLoader.h"
#include "TextureLoader/interface/TextureUtilities.h"

bool Prisma::Texture::loadTexture(const Parameters& parameters)
{
	m_parameters = parameters;
	Diligent::TextureLoadInfo loadInfo;
	loadInfo.IsSRGB = parameters.srgb;
	CreateTextureFromFile(parameters.texture.c_str(), loadInfo, Prisma::PrismaFunc::getInstance().contextData().m_pDevice, &m_texture);
	//Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,"Not found: " + m_parameters.texture);
	stbi_image_free(m_data.dataContent);
	return true;
}

uint64_t Prisma::Texture::id() const
{
	return m_id;
}

void Prisma::Texture::id(uint64_t id)
{
	m_id = id;
}

unsigned int Prisma::Texture::rawId() const
{
	return m_rawId;
}

void Prisma::Texture::rawId(unsigned int rawId)
{
	m_rawId = rawId;
}

std::string Prisma::Texture::name() const
{
	return m_name;
}

void Prisma::Texture::name(std::string name)
{
	m_name = name;
}

bool Prisma::Texture::loadCubemap(std::vector<std::string> faces, bool srgb)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		m_data.height = height;
		m_data.width = width;
		m_data.nrComponents = nrChannels;
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
			             data);
			stbi_image_free(data);
		}
		else
		{
			Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
			                                  "Cubemap texture failed to load at path: " + faces[i]);
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	m_id = glGetTextureHandleARB(textureID);
	glMakeTextureHandleResidentARB(m_id);
	m_rawId = textureID;

	return true;
}

bool Prisma::Texture::loadEquirectangular(std::string texture)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(texture.c_str(), &width, &height, &nrComponents, 0);
	m_data.height = height;
	m_data.width = width;
	m_data.nrComponents = nrComponents;
	m_parameters.texture = texture;
	m_name = m_parameters.texture;
	unsigned int hdrTexture;
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		// note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
		                                  "Failed to load HDR image.");
	}

	m_id = glGetTextureHandleARB(hdrTexture);
	glMakeTextureHandleResidentARB(m_id);
	stbi_set_flip_vertically_on_load(false);
	m_rawId = hdrTexture;
	return false;
}

Prisma::Texture::TextureData Prisma::Texture::data() const
{
	return m_data;
}

void Prisma::Texture::data(TextureData data)
{
	m_data = data;
}

void Prisma::Texture::freeData()
{
	if (m_data.deleteStbi)
	{
		if (m_data.dataContent)
		{
			stbi_image_free(m_data.dataContent);
		}
	}
	else
	{
		delete[] m_data.dataContent;
	}
}

const Prisma::Texture::Parameters Prisma::Texture::parameters() const
{
	return m_parameters;
}
