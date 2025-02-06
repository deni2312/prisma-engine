#include "../include/PlotFPS.h"
#include "implot.h"
#include <iostream>
#include <chrono>

Prisma::PlotFPS::PlotFPS() : m_startTime(std::chrono::steady_clock::now())
{
	m_fpsBuffer.reserve(m_bufferSize); // Reserve buffer size for FPS
	// Initialize the buffers with dummy data if needed
	for (int i = 0; i < m_bufferSize; i++)
	{
		m_fpsBuffer.push_back(10.0f);
	}
}

void Prisma::PlotFPS::showFPS(float fps)
{
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedTime = currentTime - m_startTime;

	if (elapsedTime.count() >= 0.2f)
	{
		UpdateFPSBuffer(fps);
		m_startTime = currentTime;
	}

	ShowFPSPlot();
}

void Prisma::PlotFPS::UpdateFPSBuffer(float fps)
{
	if (m_fpsBuffer.size() >= m_bufferSize)
	{
		m_fpsBuffer.erase(m_fpsBuffer.begin()); // Remove oldest FPS value
	}

	m_fpsBuffer.push_back(fps);

	m_min = *std::min_element(m_fpsBuffer.begin(), m_fpsBuffer.end());
	m_max = *std::max_element(m_fpsBuffer.begin(), m_fpsBuffer.end());
}

void Prisma::PlotFPS::Sparkline(const char* id, const float* values, int count, float min_v, float max_v, int offset,
                                const ImVec4& col, const ImVec2& size)
{
	if (ImPlot::BeginPlot(id, size))
	{
		ImPlot::SetupAxesLimits(0, count - 1, min_v, max_v, ImGuiCond_Always);
		ImPlot::SetNextLineStyle(col);
		ImPlot::SetNextFillStyle(col, 0.25);
		ImPlot::PlotLine(id, values, count, 1, 0, ImPlotLineFlags_Shaded, offset);
		ImPlot::EndPlot();
	}
}

void Prisma::PlotFPS::ShowFPSPlot()
{
	auto min = m_min - m_min / 20;

	auto max = m_max + m_max / 20;

	Sparkline("FPS Data", m_fpsBuffer.data(), m_fpsBuffer.size(), min, max, 0, ImPlot::GetColormapColor(1),
	          ImVec2(250, 150));
}
