#include "Postprocess/Postprocess.h"

Prisma::Postprocess::Postprocess()
{
}

void Prisma::Postprocess::render()
{
	//if (m_fbo)
	//{
	//	m_fbo->bind();
	//	for (auto effect : m_effects)
	//	{
	//		effect->render(m_fbo, m_fboRaw);
	//	}
	//	m_fbo->unbind();
	//}
	//else
	//{
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	for (auto effect : m_effects)
	//	{
	//		effect->render(m_fbo, m_fboRaw);
	//	}
	//}
}

void Prisma::Postprocess::addPostProcess(std::shared_ptr<PostprocessEffect> postprocessEffect)
{
	m_effects.push_back(postprocessEffect);
}

void Prisma::Postprocess::removePostProcess(std::shared_ptr<PostprocessEffect> postprocessEffect)
{
	// Find the effect in the m_effects list
	auto it = std::find(m_effects.begin(), m_effects.end(), postprocessEffect);

	// If found, remove it
	if (it != m_effects.end())
	{
		m_effects.erase(it);
	}
}

//void Prisma::Postprocess::fbo(std::shared_ptr<FBO> fbo)
//{
//	m_fbo = fbo;
//}
//
//void Prisma::Postprocess::fboRaw(std::shared_ptr<FBO> fbo)
//{
//	m_fboRaw = fbo;
//}
