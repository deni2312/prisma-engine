#pragma once
#include "Line.h"
#include "btBulletDynamicsCommon.h"

namespace Prisma {

    class DrawDebugger : public btIDebugDraw {
    public:
        Line line;

        void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;

        void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime,
                              const btVector3 &color) override;

        void reportErrorWarning(const char *warningString) override;

        void draw3dText(const btVector3 &location, const char *textString) override;

        void setDebugMode(int debugMode) override;

        int getDebugMode() const override;

    private:
        int debug;
    };
}