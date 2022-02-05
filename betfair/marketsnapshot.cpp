#include "marketsnapshot.h"

namespace betfair {

//=========================================================================
TMarketSnapshot::TMarketSnapshot(const QString& mkid,
                                 const std::int64_t& runner_id,
                                 const double timestamp,
                                 int bet_delay,
                                 const double& lpm,
                                 const double& adj,
                                 const double& total_vol,
                                 const QString& stat,
                                 const QString& market_stat,
                                 const double last_mk_match_ts)
    : m_market_id(mkid),     
      m_timestamp(timestamp),
      m_status(stat),
      m_market_status(market_stat),
      m_selection_id(runner_id),
      m_bet_delay(bet_delay),
      m_last_price_matched(lpm),
      m_total_matched_volume(total_vol),
      m_adj_factor(adj),
      m_last_mkt_matchtime(last_mk_match_ts)

{


}

//=========================================================================
double TMarketSnapshot::getAverageMatchedPrice() const
{
    // Get overall weighted average price matched on this runner at
    // the current point in time
    double apm = 0.0;
    if (!m_traded_history.empty())
    {
        double vol_total = 0.0;
        double weighted_sum = 0.0;
        for (auto it = m_traded_history.begin(); it != m_traded_history.end(); ++it)
        {
            vol_total += it->second;
            weighted_sum += (it->second * it->first);
        }
        if (vol_total > 0.0)
        {
            apm = weighted_sum / vol_total;
        }
    }
    if (apm > 1000.0)
    {
        apm = 1000.0;
    }
    return apm;
}

//=========================================================================
double TMarketSnapshot::getMinMatchedPrice() const
{
    double min_odds = 0.0;
    if (!m_traded_history.empty())
    {
        auto it = m_traded_history.begin();
        min_odds = it->first;
    }
    return min_odds;
}

//========================================================================
double TMarketSnapshot::getMaxMatchedPrice() const
{
    double max_odds = 0.0;
    if (!m_traded_history.empty())
    {
        // use reverse iterator for max price because
        // map sorted by ascending price
        auto it = m_traded_history.rbegin();
        max_odds = it->first;
    }
    return max_odds;
}

} // end of namespace betfair
