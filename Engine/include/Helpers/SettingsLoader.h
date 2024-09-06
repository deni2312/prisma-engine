#pragma once

#include <fstream>
#include <iostream>
#include "Settings.h"
#include "../GlobalData/InstanceData.h"

namespace Prisma {
    class SettingsLoader : public InstanceData<SettingsLoader>{
    public:
        void load(const std::string& filename);

        // Accessor method to get the loaded settings
        const Settings& getSettings() const;
        
        SettingsLoader();

    private:
        // Default settings in case of an error or when the file is not present
        void setDefaultSettings(const std::string& filename);

        // Member variable to store the loaded settings
        Settings settings;
    };
}
