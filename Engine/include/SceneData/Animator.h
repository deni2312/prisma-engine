#pragma once
#include "Animation.h"
#include "../Handlers/AnimationHandler.h"


namespace Prisma {

	class Animation;

	struct AssimpNodeData;

	class Animator {
	public:
		Animator(std::shared_ptr<Animation> animation);

		void updateAnimation(float dt);

		void playAnimation(std::shared_ptr<Animation> pAnimation);

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
	};

}