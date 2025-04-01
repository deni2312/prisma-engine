#pragma once

#include "PipelineForward.h"
#include "../SceneObjects/Camera.h"
#include "PipelineDeferred.h"
#include "../Helpers/Settings.h"
#include "../../../GUI/include/ImGuiDebug.h"
#include "PipelineSkybox.h"
#include <memory>
#include "PipelineDeferredForward.h"

namespace Prisma
{
	class PipelineHandler : public InstanceData<PipelineHandler>
	{
	public:
		struct TextureData
		{
			Diligent::RefCntAutoPtr<Diligent::ITexture> pColorRTV;
			Diligent::RefCntAutoPtr<Diligent::ITexture> pDepthDSV;
		};

		bool initScene(SceneLoader::SceneParameters sceneParameters);
		std::shared_ptr<PipelineForward> forward();
		std::shared_ptr<PipelineDeferred> deferred();
		std::shared_ptr<PipelineDeferredForward> deferredForward();
		PipelineHandler();

		TextureData textureData();

	private:
		std::shared_ptr<PipelineForward> m_forwardPipeline;
		std::shared_ptr<PipelineDeferred> m_deferredPipeline;
		std::shared_ptr<PipelineDeferredForward> m_deferredForwardPipeline;

		TextureData m_textureData;

	};
}
