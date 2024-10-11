#include "../../include/Containers/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../include/Helpers/stb_image.h"
#include "GL/glew.h"
#include "../../include/Helpers/GarbageCollector.h"
#include "../../../GUI/include/TextureInfo.h"
#include <iostream>
#include <tuple>

bool Prisma::Texture::loadTexture(const Parameters& parameters)
{
    m_parameters = parameters;
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    m_data.dataContent = stbi_load(m_parameters.texture.c_str(), &width, &height, &nrComponents, 0);
    m_data.height = height;
    m_data.width = width;
    m_data.nrComponents = nrComponents;
    if (m_data.dataContent)
    {
        GLenum internalFormat= GL_RGB;
        GLenum dataFormat= GL_RGB;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = m_parameters.srgb ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = m_parameters.srgb ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, m_data.dataContent);
        glGenerateMipmap(GL_TEXTURE_2D);
        if(m_parameters.noRepeat) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }else{
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLfloat value, max_anisotropy = 8.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &value);

        value = (value > max_anisotropy) ? max_anisotropy : value;
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, value);
        if (!m_parameters.mantainData) {
            stbi_image_free(m_data.dataContent);
        }
        if(m_parameters.resident) {
            m_id = glGetTextureHandleARB(textureID);
            glMakeTextureHandleResidentARB(m_id);
        }else{
            m_id=textureID;
        }

        Prisma::TextureInfo::getInstance().add({ textureID, m_parameters.texture });

        Prisma::GarbageCollector::getInstance().addTexture({ textureID, m_id });
        return true;
    }
    else
    {
        std::cout << "Not found: " + m_parameters.texture << std::endl;
        stbi_image_free(m_data.dataContent);
        return false;
    }
}

uint64_t Prisma::Texture::id() const
{
    return m_id;
}

void Prisma::Texture::id(uint64_t id)
{
    m_id = id;
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
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
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
    unsigned int hdrTexture;
    if (data)
    {
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }

    m_id = glGetTextureHandleARB(hdrTexture);
    glMakeTextureHandleResidentARB(m_id);
    stbi_set_flip_vertically_on_load(false);
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

void Prisma::Texture::freeData() {
    if (m_data.deleteStbi) {
        if (m_data.dataContent) {
            stbi_image_free(m_data.dataContent);
        }
    }
    else {
        delete[] m_data.dataContent;
    }
}
