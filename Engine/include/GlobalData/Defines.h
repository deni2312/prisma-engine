#pragma once
#include <string>
#include "glm/glm.hpp"


namespace Prisma {
	namespace Define {
		constexpr int MAX_DIR_LIGHTS = 1024;
		constexpr int MAX_OMNI_LIGHTS = 1024;
		constexpr int MAX_AREA_LIGHTS = 1024;
		constexpr int MAX_CLUSTER_SIZE = 100;
		constexpr int MAX_BONE_INFLUENCE = 4;
		constexpr int MAX_BONES = 128;
		constexpr int MAX_ANIMATION_MESHES = 1024;
		constexpr int MAX_SHADOW_OMNI_TEXTURE_SIZE = 1024;
		constexpr int MAX_SHADOW_DIR_TEXTURE_SIZE = 4096;
		constexpr int MAX_MESHES = 2048;
		constexpr int DEFAULT_MIPS = 4;
		const std::string DIR_DEFAULT_BLACK = "../../../Resources/res/black.png";
		const std::string DIR_DEFAULT_WHITE = "../../../Resources/res/white.jpg";
		const std::string DIR_DEFAULT_SCENE = "../../../Resources/BasicScene/BasicScene.gltf";
		const std::string DIR_DEFAULT_NORMAL = "../../../Resources/res/normal.jpg";
		const std::string DIR_DEFAULT_SETTINGS = "../../../Resources/configuration/settings.json";
		const std::string DIR_DEFAULT_LTC1 = "../../../Resources/res/ltc1.txt";
		const std::string DIR_DEFAULT_LTC2 = "../../../Resources/res/ltc2.txt";
		constexpr auto CLEAR_COLOR = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}