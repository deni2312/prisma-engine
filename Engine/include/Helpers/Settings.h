#pragma once

#include <functional>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace Prisma {
struct Settings {
        std::string name = "";
        unsigned int width = 1920;
        unsigned int height = 1080;
        bool fullscreen = false;


        // Default file paths
        std::string black_texture;
        std::string white_texture;
        std::string scene_path;
        std::string normal_map;
        std::string settings_file;
        std::string ltc1;
        std::string ltc2;

        // Rendering limits
        int max_directional_lights;
        int max_omni_lights;
        int max_omni_shadow;
        int max_area_lights;
        int max_cluster_size;
        int max_bone_influence;
        int max_bones;
        int max_animation_meshes;
        int max_shadow_omni_texture_size;
        int max_shadow_dir_texture_size;
        int max_meshes;
        int max_sprites;
        int max_raytracing_meshes;
        int default_mips;

        // Clear color RGBA
        glm::vec4 clear_color;
};

struct CallbackHandler {
        std::function<void(int width, int height)> resize;
        std::function<void(int x, int y)> mouse;
        std::function<void(int button, int action, double x, double y)> mouseClick;
        std::function<void(int key, int scancode, int action, int mods)> keyboard;
        std::function<void(double xOffset, double yOffset)> rollMouse;
};
}