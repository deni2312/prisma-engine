#pragma once
#include <vector>
#include <chrono>
#include <imgui.h>

namespace Prisma::GUI {
	class PlotFPS
	{
	public:
		PlotFPS();
		void showFPS(float fps);

	private:
		std::vector<float> m_fpsBuffer;

		const int m_bufferSize = 20; // Adjust the size of the buffer

		std::chrono::steady_clock::time_point m_startTime;

		float m_min = 0;

		float m_max = 0;

		void UpdateFPSBuffer(float fps);

		void Sparkline(const char* id, const float* values, int count, float min_v, float max_v, int offset,
		               const ImVec4& col, const ImVec2& size);

		void ShowFPSPlot();
	};
}
