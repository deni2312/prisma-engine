#pragma once
#include <memory>

#include "Postprocess/Effects.h"
#include "Postprocess/Bloom.h"


namespace Prisma::GUI {
class SettingsTab {
public:
    struct SettingsData {
        bool msaa = true;
        int numSamples = 4;
    };

    void init();
    void drawSettings();
    void updateStatus();

private:
    SettingsData m_settings;
    bool m_position = false;
    std::shared_ptr<Effects> m_effects;
    std::unique_ptr<Bloom> m_bloomRender;
    std::unique_ptr<VolumetricLight> m_volumetricRender;
};
}