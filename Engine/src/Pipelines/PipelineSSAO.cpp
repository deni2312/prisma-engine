#include "../../include/Pipelines/PipelineSSAO.h"

float Prisma::PipelineSSAO::ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}
