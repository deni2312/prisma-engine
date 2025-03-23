#pragma once
#include "PostprocessEffect.h"
#include <vector>
#include "../GlobalData/InstanceData.h"

namespace Prisma
{
	class Postprocess : public InstanceData<Postprocess>
	{
	public:
		Postprocess();

		void render();
		void addPostProcess(std::shared_ptr<PostprocessEffect> postprocessEffect);
		void removePostProcess(std::shared_ptr<PostprocessEffect> postprocessEffect);

		//void fbo(std::shared_ptr<FBO> fbo);
		//void fboRaw(std::shared_ptr<FBO> fbo);

	private:
		std::vector<std::shared_ptr<PostprocessEffect>> m_effects;
		//std::shared_ptr<FBO> m_fbo;
		//std::shared_ptr<FBO> m_fboRaw;
	};
}
