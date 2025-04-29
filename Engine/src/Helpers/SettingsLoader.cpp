#include "Helpers/SettingsLoader.h"
#include "nlohmann/json.hpp"


void to_json(nlohmann::json& j, const Prisma::Settings& s) {
        std::array<float, 4> clear_color = {s.clear_color.r, s.clear_color.g, s.clear_color.b, s.clear_color.a};

        j = nlohmann::json{{"name", s.name},
                           {"width", s.width},
                           {"height", s.height},
                           {"fullscreen", s.fullscreen},
                           {"black_texture", s.black_texture},
                           {"white_texture", s.white_texture},
                           {"scene_path", s.scene_path},
                           {"normal_map", s.normal_map},
                           {"settings_file", s.settings_file},
                           {"ltc1", s.ltc1},
                           {"ltc2", s.ltc2},
                           {"max_directional_lights", s.max_directional_lights},
                           {"max_omni_lights", s.max_omni_lights},
                           {"max_omni_shadow", s.max_omni_shadow},
                           {"max_area_lights", s.max_area_lights},
                           {"max_cluster_size", s.max_cluster_size},
                           {"max_bone_influence", s.max_bone_influence},
                           {"max_bones", s.max_bones},
                           {"max_animation_meshes", s.max_animation_meshes},
                           {"max_shadow_omni_texture_size", s.max_shadow_omni_texture_size},
                           {"max_shadow_dir_texture_size", s.max_shadow_dir_texture_size},
                           {"max_meshes", s.max_meshes},
                           {"max_sprites", s.max_sprites},
                           {"max_raytracing_meshes", s.max_raytracing_meshes},
                           {"default_mips", s.default_mips},
                           {"clear_color", clear_color}};
}

void from_json(const nlohmann::json& j, Prisma::Settings& s) {
        std::array<float, 4> clear_color;

        j.at("name").get_to(s.name);
        j.at("width").get_to(s.width);
        j.at("height").get_to(s.height);
        j.at("fullscreen").get_to(s.fullscreen);
        j.at("black_texture").get_to(s.black_texture);
        j.at("white_texture").get_to(s.white_texture);
        j.at("scene_path").get_to(s.scene_path);
        j.at("normal_map").get_to(s.normal_map);
        j.at("settings_file").get_to(s.settings_file);
        j.at("ltc1").get_to(s.ltc1);
        j.at("ltc2").get_to(s.ltc2);
        j.at("max_directional_lights").get_to(s.max_directional_lights);
        j.at("max_omni_lights").get_to(s.max_omni_lights);
        j.at("max_omni_shadow").get_to(s.max_omni_shadow);
        j.at("max_area_lights").get_to(s.max_area_lights);
        j.at("max_cluster_size").get_to(s.max_cluster_size);
        j.at("max_bone_influence").get_to(s.max_bone_influence);
        j.at("max_bones").get_to(s.max_bones);
        j.at("max_animation_meshes").get_to(s.max_animation_meshes);
        j.at("max_shadow_omni_texture_size").get_to(s.max_shadow_omni_texture_size);
        j.at("max_shadow_dir_texture_size").get_to(s.max_shadow_dir_texture_size);
        j.at("max_meshes").get_to(s.max_meshes);
        j.at("max_sprites").get_to(s.max_sprites);
        j.at("max_raytracing_meshes").get_to(s.max_raytracing_meshes);
        j.at("default_mips").get_to(s.default_mips);
        j.at("clear_color").get_to(clear_color);

        s.clear_color.r = clear_color[0];
        s.clear_color.g = clear_color[1];
        s.clear_color.b = clear_color[2];
        s.clear_color.a = clear_color[3];
}

// Load method to read settings from a file
void Prisma::SettingsLoader::load(const std::string& filename) {
        std::ifstream file(filename);

        if (file.is_open()) {
                try {
                        nlohmann::json data = nlohmann::json::parse(file);
                        from_json(data, m_settings);
                } catch (const std::exception& e) {
                        // Handle parsing errors
                        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
                        // Set default values in case of an error
                        setDefaultSettings(filename); // Pass the filename to save default settings
                }
        } else {
                // Set default values and create the config file if not open
                setDefaultSettings(filename);
        }

        file.close();
}

// Accessor method to get the loaded settings
const Prisma::Settings& Prisma::SettingsLoader::getSettings() const {
        return m_settings;
}

void Prisma::SettingsLoader::settings(Settings settings) {
        m_settings = settings;
}

Prisma::SettingsLoader::SettingsLoader() {
}

// Default settings and create the config file
void Prisma::SettingsLoader::setDefaultSettings(const std::string& filename) {
        m_settings.name = "Default";
        m_settings.width = 1920;
        m_settings.height = 1080;
        m_settings.fullscreen = false;

        // Save default settings to file
        std::ofstream defaultFile(filename);
        if (defaultFile.is_open()) {
                nlohmann::json defaultJson;
                to_json(defaultJson, m_settings);
                defaultFile << defaultJson.dump(4); // Dump with indentation for better readability
                defaultFile.close();
        } else {
                std::cerr << "Error creating default config file: " << filename << std::endl;
        }
}