#pragma once
#include <iostream>
#include "../Containers/FBO.h"

namespace Prisma {

	class PostprocessEffect {
	public:
		virtual void render(std::shared_ptr<Prisma::FBO> texture, std::shared_ptr<Prisma::FBO> raw) {
			
		};

	private:
		bool m_raw = false;
	};

}