#pragma once

namespace Prisma {

	class SettingsTab {
	public:
		struct SettingsData {
			bool msaa = true;
			int numSamples = 4;
		};
		void init();
		void drawSettings(bool& show);
	private:
		SettingsData m_settings;
	};

}