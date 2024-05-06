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

        void addTexture(std::pair<unsigned int,uint64_t> texture);

        void clear();

        GarbageCollector();

    private:
        static std::shared_ptr<GarbageCollector> instance;

        std::vector<std::pair<GarbageType,unsigned int>> m_garbage;
        std::vector<std::pair<unsigned int, uint64_t>> m_garbageTexture;
    };

}
