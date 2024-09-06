#pragma once
#include "PostprocessEffect.h"
#include "../Containers/FBO.h"
#include <vector>
#include "../GlobalData/InstanceData.h"

namespace Prisma {

	class Postprocess : public InstanceData<Postprocess>{
	public:
		Postprocess();

		void render();
		void addPostProcess(std::shared_ptr<Prisma::PostprocessEffect> postprocessEffect);

		void fbo(std::shared_ptr<Prisma::FBO> fbo);
		void fboRaw(std::shared_ptr<Prisma::FBO> fbo);

	private:
		std::vector<std::shared_ptr<Prisma::PostprocessEffect>> m_effects;
		std::shared_ptr<Prisma::FBO> m_fbo;
		std::shared_ptr<Prisma::FBO> m_fboRaw;
	};

}