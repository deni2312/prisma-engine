#pragma once
#include <string>

#include "imgui.h"

namespace Prisma
{
	class ImGuiLog
	{
	public:
		void render();

	private:
		void print(const ImVec4& color, const std::string& text);
	};
}
