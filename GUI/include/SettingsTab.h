#pragma once

namespace Prisma {

	class SettingsTab {
	public:
		struct SettingsData {
			bool msaa = true;
			int numSamples = 4;
		};
		void init();
		void drawSettings();
	private:
		SettingsData m_settings;
        bool m_position = false;
    };

}