#pragma once

#include <memory>

namespace Prisma {

    class CacheScene {
    public:
        bool updateLights();

        void updateLights(bool value);

        bool updateSizes() const;

        void updateSizes(bool value);

        bool updateData() const;

        void updateData(bool value);

        bool skipUpdate() const;

        void skipUpdate(bool value);

        bool updateTextures() const;

        void updateTextures(bool value);

        // Delete copy constructor and assignment operator
        CacheScene(const CacheScene&) = delete;

        static CacheScene& getInstance();

        void resetCaches();

        CacheScene() = default;


    private:

        static std::shared_ptr<CacheScene> instance;

        // Variables
        bool m_updateLights;
        bool m_updateSizes = false;
        bool m_updateData = false;
        bool m_skipUpdate = false;
        bool m_updateTextures = false;
    };
}
