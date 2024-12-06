#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <iostream>

namespace Prisma
{
	class FrustumCulling
	{
	public:
		FrustumCulling(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
		{
			updateFrustum(viewMatrix, projectionMatrix);
		}

		// Updates the frustum planes from the view and projection matrices
		void updateFrustum(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
		{
			glm::mat4 clipMatrix = projectionMatrix * viewMatrix; // Combine projection and view matrices

			// Extract frustum planes from the clip matrix
			planes[0] = glm::vec4(clipMatrix[0][3] + clipMatrix[0][0], // Left plane
			                      clipMatrix[1][3] + clipMatrix[1][0],
			                      clipMatrix[2][3] + clipMatrix[2][0],
			                      clipMatrix[3][3] + clipMatrix[3][0]);

			planes[1] = glm::vec4(clipMatrix[0][3] - clipMatrix[0][0], // Right plane
			                      clipMatrix[1][3] - clipMatrix[1][0],
			                      clipMatrix[2][3] - clipMatrix[2][0],
			                      clipMatrix[3][3] - clipMatrix[3][0]);

			planes[2] = glm::vec4(clipMatrix[0][3] + clipMatrix[0][1], // Bottom plane
			                      clipMatrix[1][3] + clipMatrix[1][1],
			                      clipMatrix[2][3] + clipMatrix[2][1],
			                      clipMatrix[3][3] + clipMatrix[3][1]);

			planes[3] = glm::vec4(clipMatrix[0][3] - clipMatrix[0][1], // Top plane
			                      clipMatrix[1][3] - clipMatrix[1][1],
			                      clipMatrix[2][3] - clipMatrix[2][1],
			                      clipMatrix[3][3] - clipMatrix[3][1]);

			planes[4] = glm::vec4(clipMatrix[0][3] + clipMatrix[0][2], // Near plane
			                      clipMatrix[1][3] + clipMatrix[1][2],
			                      clipMatrix[2][3] + clipMatrix[2][2],
			                      clipMatrix[3][3] + clipMatrix[3][2]);

			planes[5] = glm::vec4(clipMatrix[0][3] - clipMatrix[0][2], // Far plane
			                      clipMatrix[1][3] - clipMatrix[1][2],
			                      clipMatrix[2][3] - clipMatrix[2][2],
			                      clipMatrix[3][3] - clipMatrix[3][2]);

			// Normalize planes
			for (auto& plane : planes)
			{
				float length = glm::length(glm::vec3(plane)); // Normalize by a, b, c
				plane /= length;
			}
		}

		// Transforms the local-space AABB into world-space using the model matrix
		Prisma::Mesh::AABBssbo transformAABB(const Prisma::Mesh::AABBssbo& localAABB,
		                                     const glm::mat4& modelMatrix) const
		{
			glm::vec3 worldCenter = glm::vec3(modelMatrix * localAABB.center);

			glm::vec3 worldExtents(0.0f);
			glm::vec3 axis[3] = {
				glm::vec3(modelMatrix[0]), // X-axis
				glm::vec3(modelMatrix[1]), // Y-axis
				glm::vec3(modelMatrix[2]) // Z-axis
			};

			for (int i = 0; i < 3; ++i)
			{
				worldExtents += glm::abs(axis[i]) * glm::vec3(localAABB.extents[i]);
			}

			return Prisma::Mesh::AABBssbo{
				glm::vec4(worldCenter, 1.0f),
				glm::vec4(worldExtents, 0.0f)
			};
		}

		// Checks if the world-space AABB intersects the frustum
		bool isAABBInFrustum(const Prisma::Mesh::AABBssbo& worldAABB) const
		{
			for (const auto& plane : planes)
			{
				glm::vec3 normal(plane);
				glm::vec3 absNormal = glm::abs(normal);
				float projectedRadius = glm::dot(glm::vec3(worldAABB.extents), absNormal);
				float distance = glm::dot(normal, glm::vec3(worldAABB.center)) + plane.w;

				if (distance + projectedRadius < 0)
				{
					return false;
				}
			}
			return true;
		}

	private:
		std::array<glm::vec4, 6> planes; // 6 frustum planes
	};
}
