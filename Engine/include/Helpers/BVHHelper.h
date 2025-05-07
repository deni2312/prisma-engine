#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <numeric>
#include <vector>
#include "Pipelines/PipelineSoftwareRT.h"

namespace Prisma {
class BVHHelper {
public:
    struct AABB {
        glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

        void expand(const glm::vec3& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        void expand(const AABB& box) {
            min = glm::min(min, box.min);
            max = glm::max(max, box.max);
        }
    };

    struct BVHNode {
        AABB bounds;
        BVHNode* left = nullptr;
        BVHNode* right = nullptr;
        std::vector<int> triangleIndices;

        bool isLeaf() const { return left == nullptr && right == nullptr; }
    };

    struct BVHNodeGPU {
        glm::vec3 boundsMin;
        int leftFirst;
        glm::vec3 boundsMax;
        int count; // > 0 = leaf, == 0 = internal
    };

    struct VectorData {
        std::vector<BVHNodeGPU> flatNodes;
        std::vector<glm::ivec4> flatTriIndices;
    };


    VectorData& buildFlat(const std::vector<PipelineSoftwareRT::Vertex>& vertices, const std::vector<glm::ivec4>& indices, int maxLeafSize = 4) {
        this->vertices = &vertices;
        this->indices = &indices;

        std::vector<int> triIndices(indices.size());
        std::iota(triIndices.begin(), triIndices.end(), 0);

        m_data.flatNodes.clear();
        m_data.flatTriIndices.clear();

        BVHNode* root = buildRecursive(triIndices, maxLeafSize);
        flattenRecursive(root);
        deleteRecursive(root);
        return m_data;
    }

private:
    VectorData m_data;

    const std::vector<PipelineSoftwareRT::Vertex>* vertices;
    const std::vector<glm::ivec4>* indices;

    AABB computeTriangleBounds(int index) const {
        const auto& tri = (*indices)[index];
        auto v0 = glm::vec3((*vertices)[tri.x].vertex);
        auto v1 = glm::vec3((*vertices)[tri.y].vertex);
        auto v2 = glm::vec3((*vertices)[tri.z].vertex);

        AABB box;
        box.expand(v0);
        box.expand(v1);
        box.expand(v2);
        return box;
    }

    glm::vec3 computeTriangleCentroid(int index) const {
        const auto& tri = (*indices)[index];
        auto v0 = glm::vec3((*vertices)[tri.x].vertex);
        auto v1 = glm::vec3((*vertices)[tri.y].vertex);
        auto v2 = glm::vec3((*vertices)[tri.z].vertex);
        return (v0 + v1 + v2) / 3.0f;
    }

    BVHNode* buildRecursive(std::vector<int>& triIndices, int maxLeafSize) {
        auto node = new BVHNode();

        for (int idx : triIndices) {
            AABB triBox = computeTriangleBounds(idx);
            node->bounds.expand(triBox);
        }

        if (triIndices.size() <= maxLeafSize) {
            node->triangleIndices = triIndices;
            return node;
        }

        glm::vec3 extent = node->bounds.max - node->bounds.min;
        int axis = extent.x > extent.y ? (extent.x > extent.z ? 0 : 2) : (extent.y > extent.z ? 1 : 2);

        std::sort(triIndices.begin(), triIndices.end(), [&](int a, int b) { return computeTriangleCentroid(a)[axis] < computeTriangleCentroid(b)[axis]; });

        int mid = triIndices.size() / 2;
        std::vector<int> leftTris(triIndices.begin(), triIndices.begin() + mid);
        std::vector<int> rightTris(triIndices.begin() + mid, triIndices.end());

        node->left = buildRecursive(leftTris, maxLeafSize);
        node->right = buildRecursive(rightTris, maxLeafSize);

        return node;
    }

    int flattenRecursive(BVHNode* node) {
        int currentIndex = m_data.flatNodes.size();
        m_data.flatNodes.push_back({}); // placeholder

        BVHNodeGPU& flat = m_data.flatNodes[currentIndex];
        flat.boundsMin = node->bounds.min;
        flat.boundsMax = node->bounds.max;

        if (node->isLeaf()) {
            flat.leftFirst = m_data.flatTriIndices.size();
            flat.count = node->triangleIndices.size();
            m_data.flatTriIndices.insert(m_data.flatTriIndices.end(), node->triangleIndices.begin(), node->triangleIndices.end());
        } else {
            flat.count = 0;
            flat.leftFirst = flattenRecursive(node->left);
            flattenRecursive(node->right); // always immediately after left
        }

        return currentIndex;
    }

    void deleteRecursive(BVHNode* node) {
        if (!node) return;
        deleteRecursive(node->left);
        deleteRecursive(node->right);
        delete node;
    }
};
}