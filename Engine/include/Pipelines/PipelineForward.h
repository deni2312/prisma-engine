#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
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
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso();

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

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

		Diligent::SAMPLE_COUNT m_SupportedSampleCounts;
		int m_SampleCount;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSColorRTV;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSDepthDSV;

		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> m_pResourceSignature;


        void CreateMSAARenderTarget();

	};
}
