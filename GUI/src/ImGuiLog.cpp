#include "../include/ImGuiLog.h"

#include "imgui.h"
#include "../../Engine/include/Helpers/Logger.h"

void Prisma::ImGuiLog::render()
{
	// Get the instance of the Logger
	Prisma::Logger& logger = Prisma::Logger::getInstance();
	print(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), logger.getLogs(Prisma::LogLevel::INFO));

	print(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), logger.getLogs(Prisma::LogLevel::ERROR));

	print(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), logger.getLogs(Prisma::LogLevel::WARN));

	print(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), logger.getLogs(Prisma::LogLevel::GENERIC));
	ImGui::Dummy(ImVec2(0, 10));
}

void Prisma::ImGuiLog::print(const ImVec4& color, const std::string& text)
{
	ImGui::PushStyleColor(ImGuiCol_Text, color); // Green color

	ImGui::TextUnformatted(text.c_str());

	ImGui::PopStyleColor();
}
