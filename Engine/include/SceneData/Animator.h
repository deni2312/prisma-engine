#pragma once
#include "Animation.h"
#include "../Handlers/AnimationHandler.h"


namespace Prisma {

	class Animation;

	struct AssimpNodeData;

	class Animator {
	public:
		Animator(std::shared_ptr<Animation> animation);

		void UpdateAnimation(float dt);

		void PlayAnimation(std::shared_ptr<Animation> pAnimation);

		void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform,Prisma::AnimationHandler::SSBOAnimation& animation);

		void mesh(Prisma::Node* mesh);

	private:
		std::shared_ptr<Animation> m_CurrentAnimation;

		int findUUID();

		Prisma::Node* m_mesh;

		float m_CurrentTime;
		float m_DeltaTime;
	};

}