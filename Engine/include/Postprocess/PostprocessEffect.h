#pragma once
#include <iostream>

namespace Prisma {

	class PostprocessEffect {
	public:
		virtual void render(uint64_t texture) {
			
		};

		void raw(bool raw) {
			m_raw = raw;
		}

		bool raw() const {
			return m_raw;
		}

	private:
		bool m_raw = false;
	};

}