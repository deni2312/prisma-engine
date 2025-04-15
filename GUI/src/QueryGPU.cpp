#include "QueryGPU.h"

#include "GlobalData/PrismaFunc.h"
#include "Graphics/GraphicsEngine/interface/Query.h"
#include "Graphics/GraphicsTools/interface/ScopedQueryHelper.hpp"

Prisma::QueryGPU::QueryGPU()
{
    auto& contextData = Prisma::PrismaFunc::getInstance().contextData();

    // Check query support
    const auto& Features = contextData.m_pDevice->GetDeviceInfo().Features;
    if (Features.PipelineStatisticsQueries)
    {
        Diligent::QueryDesc queryDesc;
        queryDesc.Name = "Pipeline statistics query";
        queryDesc.Type = Diligent::QUERY_TYPE_PIPELINE_STATISTICS;
        m_pPipelineStatsQuery.reset(new Diligent::ScopedQueryHelper{ contextData.m_pDevice, queryDesc, 2 });
    }

    if (Features.OcclusionQueries)
    {
        Diligent::QueryDesc queryDesc;
        queryDesc.Name = "Occlusion query";
        queryDesc.Type = Diligent::QUERY_TYPE_OCCLUSION;
        m_pOcclusionQuery.reset(new Diligent::ScopedQueryHelper{ contextData.m_pDevice, queryDesc, 2 });
    }

    if (Features.DurationQueries)
    {
        Diligent::QueryDesc queryDesc;
        queryDesc.Name = "Duration query";
        queryDesc.Type = Diligent::QUERY_TYPE_DURATION;
        m_pDurationQuery.reset(new Diligent::ScopedQueryHelper{ contextData.m_pDevice, queryDesc, 2 });
    }

    if (Features.TimestampQueries)
    {
        m_pDurationFromTimestamps.reset(new Diligent::DurationQueryHelper{ contextData.m_pDevice, 2 });
    }
}

void Prisma::QueryGPU::start()
{
    auto immediateContext = Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext;
    // Begin supported queries
    if (m_pPipelineStatsQuery)
        m_pPipelineStatsQuery->Begin(immediateContext);
    if (m_pOcclusionQuery)
        m_pOcclusionQuery->Begin(immediateContext);
    if (m_pDurationQuery)
        m_pDurationQuery->Begin(immediateContext);
}

void Prisma::QueryGPU::end()
{
    auto immediateContext = Prisma::PrismaFunc::getInstance().contextData().m_pImmediateContext;
    if (m_pPipelineStatsQuery)
        m_pPipelineStatsQuery->End(immediateContext, &m_QueryData.PipelineStats, sizeof(m_QueryData.PipelineStats));
    if (m_pOcclusionQuery)
        m_pOcclusionQuery->End(immediateContext, &m_QueryData.Occlusion, sizeof(m_QueryData.Occlusion));
    if (m_pDurationQuery)
        m_pDurationQuery->End(immediateContext, &m_QueryData.Duration, sizeof(m_QueryData.Duration));

}

Prisma::QueryGPU::QueryData& Prisma::QueryGPU::queryData()
{
    return m_QueryData;
}
