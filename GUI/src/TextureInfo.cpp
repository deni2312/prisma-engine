#include "../include/TextureInfo.h"

#include <algorithm>

#include "GL/glew.h"
#include "../../Engine/include/GlobalData/GlobalData.h"
#include "../include/ImGuiHelper.h"

Prisma::TextureInfo::TextureInfo()
{
}

void Prisma::TextureInfo::showTextures()
{
	constexpr int numColumns = 5;
	int scale = 10;

	auto data = [&](int i) {
        if (ImGui::BeginTable("TextureTable", numColumns)) {
            for (int j = 0; j < numColumns; j++) {
                if (i * numColumns + j < Prisma::GlobalData::getInstance().globalTextures().size()) {
                    ImGui::TableNextColumn(); // Move to the next column

                    auto texture = Prisma::GlobalData::getInstance().globalTextures()[i * numColumns + j];

                    // Display the image
                    ImGui::Image((void*)static_cast<intptr_t>(texture.first), ImVec2(100, 100));

                    // Get last part of the file path
                    auto getLast = [](std::string s) {
                        size_t found = s.find_last_of('/');
                        return found != std::string::npos ? s.substr(found + 1) : s;
                        };

                    // Display the text under the image
                    ImGui::TextWrapped("%s", getLast(texture.second).c_str());
                }
                else {
                    break;
                }
            }
            ImGui::EndTable();
        }
		ImGui::NewLine();
		};
    auto size = Prisma::GlobalData::getInstance().globalTextures().size();
    if (size % numColumns > 0) {
        size = size / numColumns +1;
    }
    else {
        size=size/ numColumns;
    }
	Prisma::ImGuiHelper::getInstance().clipVertical(size, data);
    

	// End the columns
}
