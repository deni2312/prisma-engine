#pragma once

#include <memory>
#include "../GlobalData/InstanceData.h"


namespace Prisma {

    class CacheScene : public InstanceData<CacheScene>{
    public:
        bool updateLights();

        void updateLights(bool value);

        bool updateSizes() const;

        void updateSizes(bool value);

        bool updateData() const;

        void updateData(bool value);

        bool updateTextures() const;

        void updateTextures(bool value);

        bool updateStatus();

        void updateStatus(bool value);

        void resetCaches();

        CacheScene() = default;


    private:

        // Variables
        bool m_updateLights;
        bool m_updateSizes = false;
        bool m_updateData = false;
        bool m_updateTextures = false;
        bool m_updateStatus = false;
    };
}
