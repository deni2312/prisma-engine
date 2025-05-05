#include "Helpers/PrismaRender.h"
#include "GlobalData/GlobalData.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

Prisma::PrismaRender::PrismaRender() {
}

/*void Prisma::PrismaRender::renderCube()
{
	// initialize (if necessary)
	if (m_vaoCube == nullptr)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			// front face
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // top-left
			// left face
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // top-left
			-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-right
			-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
			// right face
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left
			1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // bottom-right
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-right
			1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // top-right
			1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // top-left
			1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // bottom-left
			// bottom face
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
			-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			// top face
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-right
			1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top-left
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f // top-left
		};

		m_vaoCube = std::make_shared<VAO>();
		m_vaoCube->bind();
		m_vboCube = std::make_shared<VBO>();
		m_vboCube->writeData(sizeof(vertices), vertices);
		// link vertex attributes
		m_vaoCube->addAttribPointer(0, 3, 8 * sizeof(float), nullptr);
		m_vaoCube->addAttribPointer(1, 3, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		m_vaoCube->addAttribPointer(2, 2, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		VAO::resetVao();
	}
	// render Cube
	m_vaoCube->bind();
	glDrawArrays(GL_TRIANGLES, 0, 36);
	VAO::resetVao();
}*/


/*std::shared_ptr<Prisma::Texture> Prisma::PrismaRender::renderPerlin(unsigned int width, unsigned int height)
{
	if (!m_noiseShader)
	{
		m_noiseShader = std::make_shared<Shader>("../../../Engine/Shaders/PerlinPipeline/vertex.glsl",
		                                         "../../../Engine/Shaders/PerlinPipeline/fragment.glsl");
	}

	FBO::FBOData fboData;
	fboData.width = width;
	fboData.height = height;
	fboData.internalFormat = GL_RGBA;
	fboData.internalType = GL_FLOAT;
	fboData.name = "PERLIN";
	m_noiseFbo = std::make_shared<FBO>(fboData);
	m_noiseFbo->bind();
	m_noiseShader->use();
	m_noiseShader->setVec2(m_noiseShader->getUniformPosition("resolution"), glm::vec2(width, height));
	//renderQuad();

	m_noiseFbo->unbind();
	auto textureData = new char[width * height * 4];

	m_noiseFbo->bind();
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &textureData[0]);
	m_noiseFbo->unbind();
	auto texture = std::make_shared<Texture>();
	texture->id(m_noiseFbo->texture());
	Texture::TextureData data;
	data.width = width;
	data.height = height;
	data.dataContent = (unsigned char*)textureData;
	data.deleteStbi = false;
	data.nrComponents = 4;
	texture->data(data);
	return texture;
}*/

/*void Prisma::PrismaRender::createFbo(unsigned int width, unsigned int height)
{
	if (m_data.fbo)
	{
		glDeleteFramebuffers(1, &m_data.fbo);
		glDeleteRenderbuffers(1, &m_data.rbo);
	}

	glGenFramebuffers(1, &m_data.fbo);
	glGenRenderbuffers(1, &m_data.rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_data.fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, m_data.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_data.rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::FBO, m_data.fbo});
	GarbageCollector::getInstance().add({GarbageCollector::GarbageType::RBO, m_data.rbo});
	m_data.width = width;
	m_data.height = height;
}*/


Prisma::PrismaRender::BufferData Prisma::PrismaRender::quadBuffer() {
    if (!m_initQuad) {
        // Define fullscreen quad vertices directly
        std::vector<VData> vertices = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom-left
            {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // Bottom-right
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Top-left
            {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}} // Top-right
        };

        std::vector<unsigned int> indices = {0, 1, 2, 2, 1, 3};
        // Create vertex buffer
        Diligent::BufferDesc VertBuffDesc;
        VertBuffDesc.Name = "Vertices Data";
        VertBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
        VertBuffDesc.Size = sizeof(VData) * vertices.size();

        Diligent::BufferData VBData;
        VBData.pData = vertices.data();
        VBData.DataSize = VertBuffDesc.Size;

        PrismaFunc::getInstance().contextData().device->CreateBuffer(
            VertBuffDesc, &VBData, &m_quadBufferData.vBuffer);

        // Create index buffer
        Diligent::BufferDesc IndBuffDesc;
        IndBuffDesc.Name = "Index Data";
        IndBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
        IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
        IndBuffDesc.Size = sizeof(unsigned int) * indices.size();

        Diligent::BufferData IBData;
        IBData.pData = indices.data();
        IBData.DataSize = IndBuffDesc.Size;
        m_quadBufferData.iBufferSize = indices.size();
        PrismaFunc::getInstance().contextData().device->CreateBuffer(
            IndBuffDesc, &IBData, &m_quadBufferData.iBuffer);
        m_initQuad = true;
    }
    return m_quadBufferData;
}

Prisma::PrismaRender::BufferData Prisma::PrismaRender::cubeBuffer() {
    if (!m_initCube) {
        std::vector<VData> vertices = {
            // Back face
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
            {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}},
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}},

            // Front face
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},

            // Left face
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},

            // Right face
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},

            // Bottom face
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},
            {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
            {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}},

            // Top face
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        };

        std::vector<unsigned int> indices;
        for (unsigned int i = 0; i < vertices.size(); ++i) {
            indices.push_back(i);
        }

        // Create vertex buffer
        Diligent::BufferDesc VertBuffDesc;
        VertBuffDesc.Name = "Vertices Data";
        VertBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
        VertBuffDesc.Size = sizeof(VData) * vertices.size();

        Diligent::BufferData VBData;
        VBData.pData = vertices.data();
        VBData.DataSize = VertBuffDesc.Size;

        PrismaFunc::getInstance().contextData().device->CreateBuffer(
            VertBuffDesc, &VBData, &m_cubeBufferData.vBuffer);

        // Create index buffer
        Diligent::BufferDesc IndBuffDesc;
        IndBuffDesc.Name = "Index Data";
        IndBuffDesc.Usage = Diligent::USAGE_IMMUTABLE;
        IndBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
        IndBuffDesc.Size = sizeof(unsigned int) * indices.size();

        Diligent::BufferData IBData;
        IBData.pData = indices.data();
        IBData.DataSize = IndBuffDesc.Size;
        m_cubeBufferData.iBufferSize = indices.size();
        PrismaFunc::getInstance().contextData().device->CreateBuffer(
            IndBuffDesc, &IBData, &m_cubeBufferData.iBuffer);
        m_initCube = true;
    }
    return m_cubeBufferData;
}