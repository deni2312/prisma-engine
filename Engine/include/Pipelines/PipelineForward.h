#pragma once

#include "../Helpers/Shader.h"
#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "../Containers/FBO.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include "../Physics/DrawDebugger.h"
#include <memory>
#include "PipelinePrePass.h"

namespace Prisma
{
	class PipelineForward
	{
	public:
		PipelineForward(const unsigned int& width = 1920, const unsigned int& height = 1080, bool srgb = true);
		void render();
		~PipelineForward();

	private:
		unsigned int m_width;
		unsigned int m_height;

		/*std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderAnimate;
		std::shared_ptr<Shader> m_shaderTransparent;
		std::shared_ptr<FBO> m_fbo;
		std::shared_ptr<FBO> m_fboCopy;
		std::shared_ptr<PipelineFullScreen> m_fullscreenPipeline;

		std::shared_ptr<PipelinePrePass> m_prepass;*/

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_mvpVS;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_shader;
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

	};
}
