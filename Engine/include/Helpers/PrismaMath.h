#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <Jolt/Jolt.h>

#include "glm/gtx/euler_angles.hpp"


namespace Prisma {
struct HitInfo {
    bool hit;
    glm::vec3 point;
};

constexpr auto oglToVkProjection = glm::mat4(
    1, 0, 0, 0,
    0, -1, 0, 0, // Flip Y
    0, 0, 1, 0,
    0, 0, 0, 1
    );

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

// glm::vec3 to JPH::Vec3
static JPH::Vec3 JtoVec3(const glm::vec3& v) {
    return JPH::Vec3(v.x, v.y, v.z);
}

// JPH::Vec3 to glm::vec3
static glm::vec3 JfromVec3(const JPH::Vec3& v) {
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

// glm::vec4 to JPH::Vec4
static JPH::Vec4 JtoVec4(const glm::vec4& v) {
    return JPH::Vec4(v.x, v.y, v.z, v.w);
}

// JPH::Vec4 to glm::vec4
static glm::vec4 JfromVec4(const JPH::Vec4& v) {
    return glm::vec4(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
}

// glm::mat4 to JPH::Mat4
static JPH::Mat44 JtoMat4(const glm::mat4& m) {
    JPH::Mat44 joltMat;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            joltMat(i, j) = m[i][j]; // Access elements directly by row and column
        }
    }
    return joltMat;
}

// glm::vec4 to JPH::Vec4
static JPH::Quat JtoQuat(const glm::quat& v) {
    return JPH::Quat(v.x, v.y, v.z, v.w);
}

// glm::vec4 to JPH::Vec4
static glm::quat JtoQuat(const JPH::Quat& v) {
    return glm::quat(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
}

// JPH::Mat4 to glm::mat4
static glm::mat4 JfromMat4(const JPH::Mat44& m) {
    glm::mat4 glmMat;
    auto& m1 = m.Transposed();
    // Iterate over rows and columns to copy each element
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            glmMat[i][j] = m1(i, j); // Swap indices to account for column-major order in glm
        }
    }
    return glmMat;
}

static glm::mat4 recomposeTransform(const glm::vec3& translation, const glm::vec3& rotationEuler,
                                    const glm::vec3& scale) {
    glm::mat4 translationMatrix = translate(glm::mat4(1.0f), translation);
    glm::mat4 rotationMatrix = glm::eulerAngleYXZ(rotationEuler.y, rotationEuler.x, rotationEuler.z);
    // Y -> X -> Z order
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    return translationMatrix * rotationMatrix * scaleMatrix;
}

static void decomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotationEuler,
                               glm::vec3& scale) {
    // Extract translation (last column of the matrix)
    translation = glm::vec3(transform[3]);

    // Extract scale (length of basis vectors)
    auto right = glm::vec3(transform[0]);
    auto up = glm::vec3(transform[1]);
    auto forward = glm::vec3(transform[2]);

    scale.x = length(right);
    scale.y = length(up);
    scale.z = length(forward);

    // Normalize rotation basis vectors
    auto rotationMatrix = glm::mat3(transform);
    rotationMatrix[0] /= scale.x;
    rotationMatrix[1] /= scale.y;
    rotationMatrix[2] /= scale.z;

    // Convert rotation matrix to Euler angles (YXZ order)
    rotationEuler = eulerAngles(quat_cast(rotationMatrix));
}

static glm::mat4 getTransform(aiMatrix4x4 matrix) {
    glm::mat4 transform;
    for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
            transform[j][k] = matrix[j][k];
        }
    }
    transform = transpose(transform);
    return transform;
}

static glm::vec3 getVec3(const aiVector3D& vec) {
    return glm::vec3(vec.x, vec.y, vec.z);
}

static bool containsNaN(const glm::mat4& mat) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (std::isnan(mat[i][j])) {
                return true;
            }
        }
    }
    return false;
}

static glm::quat getQuat(const aiQuaternion& pOrientation) {
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}

static glm::vec3 extractScaling(const glm::mat4& matrix) {
    // Extract the scaling factors by calculating the length of each basis vector
    glm::vec3 scale;
    scale.x = length(glm::vec3(matrix[0])); // Length of the X axis
    scale.y = length(glm::vec3(matrix[1])); // Length of the Y axis
    scale.z = length(glm::vec3(matrix[2])); // Length of the Z axis
    return scale;
}

static glm::mat3 extractRotation(const glm::mat4& matrix) {
    // Extract the basis vectors (columns 0, 1, 2) from the matrix
    auto col1 = glm::vec3(matrix[0]);
    auto col2 = glm::vec3(matrix[1]);
    auto col3 = glm::vec3(matrix[2]);

    // Normalize the columns to remove scaling and get pure rotation
    glm::vec3 normCol1 = normalize(col1);
    glm::vec3 normCol2 = normalize(col2);
    glm::vec3 normCol3 = normalize(col3);

    // Construct the rotation matrix (3x3)
    glm::mat3 rotation(normCol1, normCol2, normCol3);

    return rotation;
}


static HitInfo rayAABBIntersect(const glm::vec3& rayStart, const glm::vec3& rayEnd, const glm::vec3& bbMin,
                                const glm::vec3& bbMax) {
    glm::vec3 rayDir = normalize(rayEnd - rayStart);

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