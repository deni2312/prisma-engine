#pragma once
#include "GlobalData/InstanceData.h"
#include "GlobalData/Platform.h"
#include "Graphics/GraphicsEngine/interface/Query.h"
#include "Graphics/GraphicsTools/interface/DurationQueryHelper.hpp"
#include "Graphics/GraphicsTools/interface/ScopedQueryHelper.hpp"

namespace Prisma {
class QueryGPU : public InstanceData<QueryGPU> {
public:
        struct QueryData {
                Diligent::QueryDataPipelineStatistics PipelineStats;
                Diligent::QueryDataOcclusion Occlusion;
                Diligent::QueryDataDuration Duration;
                double TimestampBegin;
                double TimestampEnd;
                float TimestampFrequency;
        };

        QueryGPU();

        void start();
        void end();

        QueryData& queryData();

private:
        std::unique_ptr<Diligent::ScopedQueryHelper> m_pPipelineStatsQuery;
        std::unique_ptr<Diligent::ScopedQueryHelper> m_pOcclusionQuery;
        std::unique_ptr<Diligent::ScopedQueryHelper> m_pDurationQuery;
        std::unique_ptr<Diligent::ScopedQueryHelper> m_pTimestampQueryBegin;
        std::unique_ptr<Diligent::ScopedQueryHelper> m_pTimestampQueryEnd;
        std::unique_ptr<Diligent::DurationQueryHelper> m_pDurationFromTimestamps;
        double m_DurationFromTimestamps = 0;

        QueryData m_QueryData;
};
}