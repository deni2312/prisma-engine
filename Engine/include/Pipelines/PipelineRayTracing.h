#pragma once

#include "../SceneObjects/Camera.h"
#include "../SceneData/SceneLoader.h"
#include "../Handlers/MeshHandler.h"
#include "../SceneData/MeshIndirect.h"
#include "PipelineFullScreen.h"
#include "../Helpers/Settings.h"
#include <memory>
#include "PipelinePrePass.h"

namespace Prisma
{
	class PipelineRayTracing
	{
	public:
		PipelineRayTracing(const unsigned int& width = 1920, const unsigned int& height = 1080, bool srgb = true);
		void render();
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso();


		~PipelineRayTracing();

	private:
		unsigned int m_width;
		unsigned int m_height;

		struct RayTracingData
		{
			glm::vec4   CameraPos;
			glm::mat4 InvViewProj;

			// Near and far clip plane distances
			glm::vec2   ClipPlanes;
			float MaxRecursion;
			float   Padding0;
		};

		/*std::shared_ptr<Shader> m_shader;
		std::shared_ptr<Shader> m_shaderAnimate;
		std::shared_ptr<Shader> m_shaderTransparent;
		std::shared_ptr<FBO> m_fbo;
		std::shared_ptr<FBO> m_fboCopy;
		std::shared_ptr<PipelineFullScreen> m_fullscreenPipeline;

		std::shared_ptr<PipelinePrePass> m_prepass;*/

		Diligent::Uint32 m_MaxRecursionDepth = 8;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pso;

		Diligent::SAMPLE_COUNT m_SupportedSampleCounts;
		int m_SampleCount;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSColorRTV;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> m_pMSDepthDSV;

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> m_srb;

		std::function<void()> m_updateData;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> m_raytracingData;

		Diligent::RefCntAutoPtr<Diligent::ITexture> m_colorBuffer;

		Prisma::Settings m_settings;

	};
}
