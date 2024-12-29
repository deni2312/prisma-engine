#pragma once
#include "../../../Engine/include/Components/Component.h"
#include "../Postprocess/include/Shockwave.h"

class ShockwaveComponent : public Prisma::Component {
public:
	ShockwaveComponent();
	void start() override;
	void update() override;
private:
	std::shared_ptr<Shockwave> m_shockwave;
	Prisma::TimeCounter m_counter;
};