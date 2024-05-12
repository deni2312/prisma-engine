#pragma once
#include "../../../Engine/include/Postprocess/PostprocessEffect.h"
#include "../../../Engine/include/Helpers/Shader.h"
#include "../../../Engine/include/Helpers/IBLBuilder.h"

namespace Prisma {

	class Effects : public Prisma::PostprocessEffect{
	public:

		enum class EFFECTS {
			NORMAL,
			SEPPIA,
			CARTOON,
			VIGNETTE,
			BLOOM
		};

		Effects() {
			m_shaderSeppia = std::make_shared<Shader>("../../../GUI/Shaders/Seppia/vertex.glsl", "../../../GUI/Shaders/Seppia/fragment.glsl");
			m_shaderSeppia->use();
			m_bindlessPosSeppia = m_shaderSeppia->getUniformPosition("screenTexture");

			m_shaderCartoon = std::make_shared<Shader>("../../../GUI/Shaders/Cartoon/vertex.glsl", "../../../GUI/Shaders/Cartoon/fragment.glsl");
			m_shaderCartoon->use();
			m_bindlessPosCartoon = m_shaderCartoon->getUniformPosition("screenTexture");

			m_shaderVignette = std::make_shared<Shader>("../../../GUI/Shaders/Vignette/vertex.glsl", "../../../GUI/Shaders/Vignette/fragment.glsl");
			m_shaderVignette->use();
			m_bindlessPosVignette = m_shaderVignette->getUniformPosition("screenTexture");

			m_shaderBloom = std::make_shared<Shader>("../../../GUI/Shaders/Bloom/vertex.glsl", "../../../GUI/Shaders/Bloom/fragment.glsl");
			m_shaderBloom->use();
			m_bindlessPosBloom = m_shaderBloom->getUniformPosition("screenTexture");
		}

		void effect(EFFECTS effect) {
			m_effects = effect;
		}


		virtual void render(uint64_t texture) override {
			raw(false);
			switch (m_effects) {
			case EFFECTS::NORMAL:
				break;
			case EFFECTS::SEPPIA:
				m_shaderSeppia->use();
				m_shaderSeppia->setInt64(m_bindlessPosSeppia, texture);
				Prisma::IBLBuilder::getInstance().renderQuad();
				break;
			case EFFECTS::CARTOON:
				m_shaderCartoon->use();
				m_shaderCartoon->setInt64(m_bindlessPosCartoon, texture);
				Prisma::IBLBuilder::getInstance().renderQuad();
				break;
			case EFFECTS::VIGNETTE:
				m_shaderVignette->use();
				m_shaderVignette->setInt64(m_bindlessPosVignette, texture);
				Prisma::IBLBuilder::getInstance().renderQuad();
				break;
			case EFFECTS::BLOOM:
				raw(true);
				m_shaderBloom->use();
				m_shaderBloom->setInt64(m_bindlessPosBloom, texture);
				Prisma::IBLBuilder::getInstance().renderQuad();
				break;
			}
		}
	private:
		std::shared_ptr<Shader> m_shaderSeppia;
		unsigned int m_bindlessPosSeppia;

		std::shared_ptr<Shader> m_shaderCartoon;
		unsigned int m_bindlessPosCartoon;

		std::shared_ptr<Shader> m_shaderVignette;
		unsigned int m_bindlessPosVignette;

		std::shared_ptr<Shader> m_shaderBloom;
		unsigned int m_bindlessPosBloom;

		EFFECTS m_effects = EFFECTS::NORMAL;
	};

}