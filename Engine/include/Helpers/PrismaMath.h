#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include "bullet/btBulletDynamicsCommon.h"

namespace Prisma{

    struct HitInfo {
        bool hit;
        glm::vec3 point;
    };

    static bool mat4Equals(const glm::mat4& a, const glm::mat4& b, float epsilon = 0.0001f) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                if (glm::abs(a[i][j] - b[i][j]) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }

    static glm::mat4 createModelMatrix(const glm::vec3& translation, const glm::mat4& rotation, const glm::vec3& scale) {
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) *
                               rotation *
                                glm::scale(glm::mat4(1.0f), scale);
        return modelMatrix;
    }

    static glm::quat eulerToQuaternion(float roll, float pitch, float yaw) {
        // Convert Euler angles to radians
        roll = glm::radians(roll);
        pitch = glm::radians(pitch);
        yaw = glm::radians(yaw);

        glm::quat q;
        q = glm::quat(glm::vec3(pitch, yaw, roll));  // GLM expects pitch, yaw, roll order

        return q;
    }

    static glm::mat4 getTransform(aiMatrix4x4 matrix) {
        glm::mat4 transform;
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                transform[j][k] = matrix[j][k];
            }
        }
        transform = glm::transpose(transform);
        return transform;
    }

    static glm::vec3 getVec3(const aiVector3D& vec)
    {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    static glm::vec3 getVec3GLM(const btVector3& vec)
    {
        return glm::vec3(vec.getX(), vec.getY(), vec.getZ());
    }

    static btVector3 getVec3BT(const glm::vec3& vec)
    {
        return btVector3(vec.x, vec.y, vec.z);
    }

    static glm::quat getQuat(const aiQuaternion& pOrientation)
    {
        return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
    }

    static glm::vec3 extractScaling(const glm::mat4& matrix) {
        // Extract the scaling factors by calculating the length of each basis vector
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(matrix[0])); // Length of the X axis
        scale.y = glm::length(glm::vec3(matrix[1])); // Length of the Y axis
        scale.z = glm::length(glm::vec3(matrix[2])); // Length of the Z axis
        return scale;
    }

    static glm::mat3 extractRotation(const glm::mat4& matrix) {
        // Extract the basis vectors (columns 0, 1, 2) from the matrix
        glm::vec3 col1 = glm::vec3(matrix[0]);
        glm::vec3 col2 = glm::vec3(matrix[1]);
        glm::vec3 col3 = glm::vec3(matrix[2]);

        // Normalize the columns to remove scaling and get pure rotation
        glm::vec3 normCol1 = glm::normalize(col1);
        glm::vec3 normCol2 = glm::normalize(col2);
        glm::vec3 normCol3 = glm::normalize(col3);

        // Construct the rotation matrix (3x3)
        glm::mat3 rotation(normCol1, normCol2, normCol3);

        return rotation;
    }


    static HitInfo rayAABBIntersect(const glm::vec3& rayStart, const glm::vec3& rayEnd, const glm::vec3& bbMin, const glm::vec3& bbMax) {
        glm::vec3 rayDir = glm::normalize(rayEnd - rayStart);

        float tmin = (bbMin.x - rayStart.x) / rayDir.x;
        float tmax = (bbMax.x - rayStart.x) / rayDir.x;

        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (bbMin.y - rayStart.y) / rayDir.y;
        float tymax = (bbMax.y - rayStart.y) / rayDir.y;

        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) return {false, glm::vec3(0)};

        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (bbMin.z - rayStart.z) / rayDir.z;
        float tzmax = (bbMax.z - rayStart.z) / rayDir.z;

        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) return {false, glm::vec3(0)};

        // Now we have an intersection
        float tHit = tmin > 0 ? tmin : tmax;
        glm::vec3 hitPoint = rayStart + tHit * rayDir;

        return {true, hitPoint};
    }

}
