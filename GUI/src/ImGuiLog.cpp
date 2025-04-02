#include "../include/ImGuiLog.h"

#include "Helpers/Logger.h"
#include "ThirdParty/imgui/imgui.h"


void Prisma::ImGuiLog::render()
{
	// Get the instance of the Logger
	Prisma::Logger& logger = Prisma::Logger::getInstance();
	print(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), logger.getLogs(Prisma::LogLevel::INFO));

	print(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), logger.getLogs(Prisma::LogLevel::ERROR));

	print(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), logger.getLogs(Prisma::LogLevel::WARN));

	print(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), logger.getLogs(Prisma::LogLevel::GENERIC));
	ImGui::Dummy(ImVec2(0, 10));
}

void Prisma::ImGuiLog::print(const glm::vec4& color, const std::string& text)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.x, color.y, color.z, color.w)); // Green color

	ImGui::TextUnformatted(text.c_str());

	ImGui::PopStyleColor();
}
