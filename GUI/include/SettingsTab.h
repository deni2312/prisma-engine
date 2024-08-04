#pragma once
#include <memory>
#include "Postprocess/Effects.h"

namespace Prisma {

	class SettingsTab {
	public:
		struct SettingsData {
			bool msaa = true;
			int numSamples = 4;
		};
		void init();
		void drawSettings();
		void updateStatus();
	private:
		SettingsData m_settings;
        bool m_position = false;
		std::shared_ptr<Prisma::Effects> m_effects;
		std::shared_ptr<Prisma::Effects> m_effectsBloom;
		bool m_bloom = false;
    };

}