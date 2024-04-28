#pragma once
#include "Animation.h"


namespace Prisma {

	class Animation;

	struct AssimpNodeData;

	class Animator {
	public:
		Animator(std::shared_ptr<Animation> animation);

		void UpdateAnimation(float dt);

		void PlayAnimation(std::shared_ptr<Animation> pAnimation);

		void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

		void mesh(Prisma::Node* mesh);

		std::vector<glm::mat4> GetFinalBoneMatrices();

	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		std::shared_ptr<Animation> m_CurrentAnimation;

		Prisma::Node* m_mesh;

		float m_CurrentTime;
		float m_DeltaTime;
	};

}