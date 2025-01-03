#pragma once

#include "GL/glew.h"
#include <memory>
#include <vector>
#include "../Helpers/Shader.h"

namespace Prisma
{
	class FBO
	{
	public:
		struct FBOData
		{
			unsigned int width = 1920;
			unsigned int height = 1080;
			unsigned int internalFormat = GL_RGBA;
			unsigned int internalType = GL_UNSIGNED_INT;
			bool enableDepth = false;
			bool enableMultisample = false;
			bool enableColor = true;
			bool rbo = true;
			unsigned int filtering = GL_LINEAR;
			unsigned int border = GL_CLAMP_TO_BORDER;
			std::string name = "";
		};

		FBO(FBOData fboData);

		FBO(std::vector<FBOData> fboData);

		~FBO();

		void bind();
		void unbind();

		uint64_t texture() const;

		std::vector<uint64_t> textures() const;

		uint64_t depth() const;

		unsigned int frameBufferID();

	private:
		unsigned int m_framebufferID;
		uint64_t m_id = -1;
		std::vector<uint64_t> m_textureId;
		uint64_t m_depthId = -1;
		std::shared_ptr<Shader> m_shader;
		unsigned int m_vao;
		FBOData m_fboData;
	};
}
