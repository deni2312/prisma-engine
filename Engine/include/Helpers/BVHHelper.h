#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <numeric>
#include <vector>
#include "Pipelines/PipelineSoftwareRT.h"

namespace Prisma {
// -------- BVH Class --------
class BVH {
public:
    struct Triangle {
        glm::vec4 v0, v1, v2;
        glm::vec4 index;
    };

    // -------- AABB --------
    struct AABB {
        glm::vec4 min = glm::vec4(std::numeric_limits<float>::max());
        glm::vec4 max = glm::vec4(std::numeric_limits<float>::lowest());

        void expand(const glm::vec4& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        void expand(const AABB& box) {
            expand(box.min);
            expand(box.max);
        }

        glm::vec3 centroid() const { return (min + max) * 0.5f; }
    };

    // -------- Flat BVH Node (GPU-friendly) --------
    struct BVHNode {
        AABB bounds;
        glm::vec4 leftFirst; // index to child (internal) or primitive offset (leaf)
        glm::vec4 count; // if 0 = internal, >0 = leaf with count triangles
    };

    BVH(const std::vector<PipelineSoftwareRT::Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<PipelineSoftwareRT::Sizes>& sizes) {
        for (int j = 0; j < sizes.size(); j++) {
            auto size = sizes[j];
            // Convert triangles
            for (size_t i = size.indexBase; i < size.indexBase + size.indexSize; i += 3) {
                Triangle tri;
                tri.v0 = vertices[size.vertexBase + indices[i + 0]].vertex;
                tri.v1 = vertices[size.vertexBase + indices[i + 1]].vertex;
                tri.v2 = vertices[size.vertexBase + indices[i + 2]].vertex;
                tri.index = glm::vec4(j);
                triangles.push_back(tri);
            }
        }

        triangleIndices.resize(triangles.size());
        for (int i = 0; i < triangleIndices.size(); ++i) triangleIndices[i] = i;

        buildRecursive(0, static_cast<int>(triangles.size()));
        flatten(0);
    }

    const std::vector<BVHNode>& getFlatNodes() const { return flatNodes; }
    const std::vector<Triangle>& getTriangles() const { return triangles; }

private:
    struct Node {
        AABB bounds;
        int left = -1, right = -1;
        int start = 0, count = 0;
        bool isLeaf() const { return count > 0; }
    };

    std::vector<Triangle> triangles;
    std::vector<int> triangleIndices;
    std::vector<Node> nodes;
    std::vector<BVHNode> flatNodes;

    int buildRecursive(int start, int end) {
        Node node;
        node.start = start;
        node.count = end - start;

        // Compute bounding box
        for (int i = start; i < end; ++i) {
            const Triangle& tri = triangles[triangleIndices[i]];
            AABB triBounds;
            triBounds.expand(tri.v0);
            triBounds.expand(tri.v1);
            triBounds.expand(tri.v2);
            node.bounds.expand(triBounds);
        }

        int nodeIndex = static_cast<int>(nodes.size());
        nodes.push_back(node);

        if (node.count <= 2) return nodeIndex; // Leaf

        // Split axis using centroid
        AABB centroidBounds;
        for (int i = start; i < end; ++i) {
            const Triangle& tri = triangles[triangleIndices[i]];
            glm::vec4 c = (tri.v0 + tri.v1 + tri.v2) / 3.0f;
            centroidBounds.expand(c);
        }

        glm::vec3 diag = centroidBounds.max - centroidBounds.min;
        int axis = 0;
        if (diag.y > diag.x) axis = 1;
        if (diag.z > diag[axis]) axis = 2;

        float split = 0.5f * (centroidBounds.min[axis] + centroidBounds.max[axis]);

        // Partition
        int mid = start;
        for (int i = start; i < end; ++i) {
            const Triangle& tri = triangles[triangleIndices[i]];
            glm::vec3 c = (tri.v0 + tri.v1 + tri.v2) / 3.0f;
            if (c[axis] < split) std::swap(triangleIndices[i], triangleIndices[mid++]);
        }

        if (mid == start || mid == end) mid = (start + end) / 2;

        int left = buildRecursive(start, mid);
        int right = buildRecursive(mid, end);

        nodes[nodeIndex].left = left;
        nodes[nodeIndex].right = right;
        nodes[nodeIndex].count = 0; // mark as internal

        return nodeIndex;
    }

    int flatten(int nodeIndex) {
        const Node& node = nodes[nodeIndex];
        int flatIndex = static_cast<int>(flatNodes.size());
        flatNodes.emplace_back();

        BVHNode& flat = flatNodes.back();
        flat.bounds = node.bounds;

        if (node.isLeaf()) {
            flat.leftFirst.r = node.start;
            flat.count.r = node.count;
        } else {
            flat.count.r = 0;
            flat.leftFirst.r = flatten(node.left);
            flatten(node.right);
        }

        return flatIndex;
    }
};
}