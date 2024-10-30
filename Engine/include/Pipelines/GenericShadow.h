#pragma once
#include "glm/glm.hpp"

namespace Prisma
{
	class GenericShadow
	{
	public:
		virtual uint64_t id() =0;

		virtual float farPlane() = 0;
		virtual void farPlane(float farPlane) = 0;

		virtual float nearPlane() = 0;
		virtual void nearPlane(float nearPlane) = 0;

		virtual void update(glm::vec3 lightPos) =0;

		virtual ~GenericShadow()
		{
		} // Virtual destructor
	};
}
