#include "Helpers/SettingsLoader.h"
#include "nlohmann/json.hpp"


void to_json(nlohmann::json& j, const Prisma::Settings& s) {
    j = nlohmann::json{{"name", s.name}, {"width", s.width}, {"height", s.height}, {"fullscreen", s.fullscreen}};
}

void from_json(const nlohmann::json& j, Prisma::Settings& s) {
    j.at("name").get_to(s.name);
    j.at("width").get_to(s.width);
    j.at("height").get_to(s.height);
    j.at("fullscreen").get_to(s.fullscreen);
    j.at("vsync").get_to(s.vsync);
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