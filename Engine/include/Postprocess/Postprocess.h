#pragma once
#include "PostprocessEffect.h"
#include "../Containers/FBO.h"
#include <vector>

namespace Prisma {

	class Postprocess {
	public:
		static Postprocess& getInstance();

		Postprocess(const Postprocess&) = delete;
		Postprocess& operator=(const Postprocess&) = delete;

		Postprocess();

		void render();
		void addPostProcess(std::shared_ptr<Prisma::PostprocessEffect> postprocessEffect);

		void fbo(std::shared_ptr<Prisma::FBO> fbo);
		void fboRaw(std::shared_ptr<Prisma::FBO> fbo);

	private:
		std::vector<std::shared_ptr<Prisma::PostprocessEffect>> m_effects;
		std::shared_ptr<Prisma::FBO> m_fbo;
		std::shared_ptr<Prisma::FBO> m_fboRaw;
		static std::shared_ptr<Postprocess> instance;
	};

}