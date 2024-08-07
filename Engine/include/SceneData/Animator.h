#pragma once
#include "Animation.h"
#include "../Handlers/AnimationHandler.h"
#include <glm/gtx/matrix_decompose.hpp>


namespace Prisma {

	class Animation;

	struct AssimpNodeData;

	class Animator {
	public:
		Animator(std::shared_ptr<Animation> animation);

		void updateAnimation(float dt);

		void playAnimation(std::shared_ptr<Animation> pAnimation,float blendDuration);

		void calculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform,Prisma::AnimationHandler::SSBOAnimation& animation);

		void mesh(Prisma::Node* mesh);

		void frame(float frame);

		std::shared_ptr<Animation> animation();

		float currentTime() const;

	private:
		std::shared_ptr<Animation> m_CurrentAnimation;

		int findUUID();

		Prisma::Node* m_mesh;

		float m_CurrentTime;

		std::shared_ptr<Animation> m_PreviousAnimation;

		float m_BlendFactor = 0.0f;

		float m_BlendDuration = 0.8f;  // Duration of the blend in seconds

		bool m_IsBlending = false;

		void blendAnimations(float blendFactor);

		void calculateCurrentTransform(const AssimpNodeData* node, const glm::mat4& parentTransform);

		glm::mat4 decomposeAndInterpolateMat4(const glm::mat4& matA, const glm::mat4& matB, float t);

		glm::mat4 m_currentTransform[MAX_BONES];

		glm::mat4 m_previousTransform[MAX_BONES];
	};

}