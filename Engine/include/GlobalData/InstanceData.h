#pragma once

namespace Prisma {
template <typename T>
class InstanceData {
public:
        InstanceData(const InstanceData&) = delete;
        InstanceData& operator=(const InstanceData&) = delete;

        static T& getInstance() {
                static T instance;
                return instance;
        }

protected:
        InstanceData() = default;
        ~InstanceData() = default;
};
}