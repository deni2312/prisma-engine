#pragma once
#include <iostream>

namespace Prisma {
class PostprocessEffect {
public:
        virtual void render() = 0;
};
}