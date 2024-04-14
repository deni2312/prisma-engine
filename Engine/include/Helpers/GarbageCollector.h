#pragma once
#include <memory>
#include <vector>

namespace Prisma {

    class GarbageCollector {
    public:

        enum GarbageType{
            VAO,
            BUFFER,
            TEXTURE,
            FBO,
            RBO
        };

        GarbageCollector(const GarbageCollector &) = delete;

        GarbageCollector &operator=(const GarbageCollector &) = delete;

        static GarbageCollector &getInstance();

        void add(std::pair<GarbageType,unsigned int> garbage);

        void clear();

        GarbageCollector();

    private:
        static std::shared_ptr<GarbageCollector> instance;

        std::vector<std::pair<GarbageType,unsigned int>> m_garbage;
    };

}
