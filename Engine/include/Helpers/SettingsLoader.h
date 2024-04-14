#pragma once

#include <fstream>
#include <iostream>
#include "Settings.h"

namespace Prisma {
    class SettingsLoader {
    public:
        // Static method to access the singleton instance
        static SettingsLoader& instance();

        // Load method to read settings from a file
        void load(const std::string& filename);

        // Accessor method to get the loaded settings
        const Settings& getSettings() const;

    private:
        // Private constructor to enforce singleton pattern
        SettingsLoader() = default;

        // Private copy constructor and assignment operator to prevent copies of the singleton
        SettingsLoader(const SettingsLoader&) = delete;
        SettingsLoader& operator=(const SettingsLoader&) = delete;

        // Default settings in case of an error or when the file is not present
        void setDefaultSettings(const std::string& filename);

        // Member variable to store the loaded settings
        Settings settings;
    };
}
