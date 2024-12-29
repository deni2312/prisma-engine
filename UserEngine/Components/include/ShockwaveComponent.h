#pragma once
#include "../../../Engine/include/Components/Component.h"

class ShockwaveComponent : public Prisma::Component {
public:
	void start() override;
	void update() override;
};