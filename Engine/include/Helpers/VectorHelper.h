#pragma once
#include "../GlobalData/InstanceData.h"
#include <vector>
#include <memory>

namespace Prisma {
class VectorHelper : public InstanceData<VectorHelper> {
public:
        template <typename T>
        void remove(std::vector<std::shared_ptr<T>>& nodes, uint64_t uuid) {
                auto find = std::find_if(nodes.begin(),
                                         nodes.end(),
                                         [uuid](auto node) {
                                                 if (node->uuid() == uuid) {
                                                         return true;
                                                 }
                                                 return false;
                                         });
                if (find != nodes.end()) {
                        nodes.erase(find);
                }
        }
};
}