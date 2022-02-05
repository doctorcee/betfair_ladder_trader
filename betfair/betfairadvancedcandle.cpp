#include "betfairadvancedcandle.h"
#include "betfair/betfair_utils.h"

static const std::map<double,std::uint32_t> odds_to_tick_map = betfair::utils::generatePriceDoubleTickMap();

namespace betfair {
namespace utils {

//==================================================================
TBetfairAdvancedCandle::TBetfairAdvancedCandle(const std::int64_t& id)
    : selection_id(id),
      cumulative_vwap(0.0),
      open(0.0),
      close(0.0),
      high(0.0),
      low(0.0),
      candle_vwap(0.0),
      candle_volume(0.0),
      lpm_as_tick(0.0),
      cumul_vwap_as_tick(0.0),
      candle_vwap_as_tick(0.0),
      global_max_traded(0.0),
      global_min_traded(0.0),
      cumulative_runner_match_total(0.0),
      best_lay_price_available(0.0),
      best_back_price_available(0.0),
      market_version(0),
      start_timestamp(0.0),
      end_timestamp(0.0),
      selection_status(""),
      market_status(""),
      market_id(""),
      market_in_play_at_start(false),
      market_in_play_at_end(false),
      market_cross_matched(false)
{

}

//==================================================================
TBetfairAdvancedCandle::TBetfairAdvancedCandle(const std::int64_t& id,
                                               const std::shared_ptr<betfair::TMarketSnapshot>& a,
                                               const std::shared_ptr<betfair::TMarketSnapshot>& b)
    : selection_id(id),
      cumulative_vwap(0.0),
      open(0.0),
      close(0.0),
      high(0.0),
      low(0.0),
      candle_vwap(0.0),
      candle_volume(0.0),
      lpm_as_tick(0.0),
      cumul_vwap_as_tick(0.0),
      candle_vwap_as_tick(0.0),
      global_max_traded(0.0),
      global_min_traded(0.0),
      cumulative_runner_match_total(0.0),
      best_lay_price_available(0.0),
      best_back_price_available(0.0),
      market_version(0),
      start_timestamp(0.0),
      end_timestamp(0.0),
      selection_status(""),
      market_status(""),
      market_id(""),
      market_in_play_at_start(false),
      market_in_play_at_end(false),
      market_cross_matched(false)
{
    if (a && b)
    {
        if ((a->getTimestamp() < b->getTimestamp())
         && (a->getSelectionID() == selection_id)
         && (b->getSelectionID() == selection_id))
        {
            market_in_play_at_end = (b->getBetDelay() > 0);
            market_in_play_at_start = (a->getBetDelay() > 0);
            cumulative_runner_match_total = b->getTotalVolumeMatched();
            end_timestamp = b->getTimestamp();
            start_timestamp = a->getTimestamp();
            selection_status = b->getStatus();
            market_id = b->getMarketID();
            market_status = b->getMarketStatus();
            const std::map<double,double>& a_hist = a->getTradeHistory();
            const std::map<double,double>& b_hist = b->getTradeHistory();

            for (auto it = b_hist.begin(); it != b_hist.end(); ++it)
            {
                // For each price search for the same price in a
                const double this_price = it->first;
                const double this_vol = it->second;
                auto ait = a_hist.find(this_price);
                if (ait != a_hist.end())
                {
                    // This price is present in a - calculate the difference
                    const double old_vol = ait->second;
                    const double vol_diff = this_vol - old_vol;
                    if (vol_diff > 0.0)
                    {
                        m_traded_volume.insert(std::make_pair(this_price,vol_diff));
                    }
                    else if (vol_diff < 0.0)
                    {
                        // Could be rounding error. Just omit this price                        
                    }
                }
                else
                {
                    // not in a - therefore this is a first time price match
                    if (this_vol > 0.0)
                    {
                        m_traded_volume.insert(std::make_pair(this_price,this_vol));
                    }
                }
            }
            double wsum = 0.0;
            if (m_traded_volume.empty() == false)
            {
                low = 1500.0;
                for (auto pit = m_traded_volume.begin(); pit != m_traded_volume.end(); ++pit)
                {
                    const double p = pit->first;
                    const double v = pit->second;
                    if (p > 0.0 && v > 0.0)
                    {
                        candle_volume += v;
                        wsum += (p * v);
                        if (p < low)
                        {
                            low = p;
                        }
                        if (p > high)
                        {
                            high = p;
                        }
                    }
                }
            }
            if (candle_volume > 0.0)
            {
                candle_vwap = wsum/candle_volume;
                if (low == high)
                {
                    candle_vwap_as_tick = calculateTickFractionFromOdds(low,low);
                }
                else
                {
                    candle_vwap_as_tick = calculateTickFractionFromOdds(low,candle_vwap);
                }
            }

            // now get back prices available
            const std::map<double,double>& b_back_market = b->getBackOrders();

            // Use reverse iterator for backs becose map is sorted by key (odds)
            // and best (highest) odds are at the end
            best_back_price_available = b_back_market.empty() ? 0.0 : b_back_market.rbegin()->first;

            const std::map<double,double>& b_lay_market = b->getLayOrders();
            best_lay_price_available = b_lay_market.empty() ? 0.0 : b_lay_market.rbegin()->first;

            open = a->getLPM();
            close = b->getLPM();
            cumulative_vwap = b->getAverageMatchedPrice();
            cumul_vwap_as_tick = 0.0;
            lpm_as_tick = 0.0;
            auto it = odds_to_tick_map.find(close);
            if (it != odds_to_tick_map.end())
            {
                lpm_as_tick = it->second;
            }
            if (cumulative_vwap > 0.0)
            {
                double low = cumulative_vwap > open ? open : 1.01;
                cumul_vwap_as_tick = calculateTickFractionFromOdds(low,cumulative_vwap);
            }

            global_max_traded = b->getMaxMatchedPrice();
            global_min_traded = b->getMinMatchedPrice();
        }
    }
}

//==================================================================
double TBetfairAdvancedCandle::calculateTickFractionFromOdds(const double& low,
                                                             const double& vwap)
{
    double val = 0.0;
    auto low_it = odds_to_tick_map.find(low);
    if (low_it != odds_to_tick_map.end())
    {
        if (low == vwap)
        {
            val = static_cast<double>(low_it->second);
        }
        else
        {
            std::uint32_t tick_under = low_it->second;
            double odds_under = low_it->first;
            double odds_above = 0.0;
            while (low_it != odds_to_tick_map.end())
            {
                if (vwap > low_it->first)
                {
                    tick_under = low_it->second;
                }
                else
                {
                    odds_above = low_it->first;
                    break;
                }
                ++low_it;
            }
            if ((odds_above > 0.0) && (vwap >= odds_under) && (vwap <= odds_above))
            {
                const double width = (odds_above - odds_under);
                const double ratio = (candle_vwap - odds_under) / width;
                val = tick_under + ratio;
            }
            else
            {
                throw std::runtime_error("TBetfairAdvancedCandle::calculateTickFractionFromOdds() Error in tick calculation!");
            }
        }
    }
    return val;
}

//==================================================================
void TBetfairAdvancedCandle::calculateCandleRangeAndMean(const double& candle_min_vol_threshold,
                                                           double& low,
                                                           double& high,
                                                           double& volume,
                                                           double& weighted_avg_match) const
{
    double wsum = 0.0;
    weighted_avg_match = 0.0;
    volume = 0.0;
    low = 0.0;
    high = 0.0;
    if (m_traded_volume.empty() == false)
    {
        low = 1500.0;
        for (auto pit = m_traded_volume.begin(); pit != m_traded_volume.end(); ++pit)
        {
            const double p = pit->first;
            const double v = pit->second;
            if (p > 0.0 && v > candle_min_vol_threshold)
            {
                volume += v;
                wsum += (p * v);
                if (p < low)
                {
                    low = p;
                }
                if (p > high)
                {
                    high = p;
                }
            }
        }
    }
    if (volume > 0.0)
    {
        weighted_avg_match = wsum/volume;
    }
}

//==================================================================
void TBetfairAdvancedCandle::merge(const TBetfairAdvancedCandle& candle)
{
    if (selection_id != candle.selection_id)
    {
        throw std::runtime_error("TBetfairAdvancedCandle::merge() selection ID mismatch!");
    }
    if (market_id != candle.market_id)
    {
        throw std::runtime_error("TBetfairAdvancedCandle::merge() market ID mismatch!");
    }
    if (candle.start_timestamp < start_timestamp)
    {
        throw std::runtime_error("TBetfairAdvancedCandle::merge() Candle to merge has earlier timestamp than this candle!");
    }

    // Assumes that candle argument is LATER candle than *this
    cumulative_vwap = candle.cumulative_vwap;
    global_max_traded = candle.global_max_traded;
    global_min_traded = candle.global_min_traded;
    cumulative_runner_match_total = candle.cumulative_runner_match_total;
    // open will NOT change
    close = candle.close;
    best_lay_price_available = candle.best_lay_price_available;
    best_back_price_available = candle.best_back_price_available;

    market_version = candle.market_version;
    end_timestamp = candle.end_timestamp;
    selection_status = candle.selection_status;
    market_status = candle.market_status;
    market_in_play_at_end = candle.market_in_play_at_end;
    market_cross_matched = candle.market_cross_matched;

    // Iterate over NEW candle prices. Add new volumes to existing volumes for prices that exist in both
    // THIS and the new candle and insert volumes present in NEW candle but not in THIS
    for (auto pit = candle.m_traded_volume.begin(); pit != candle.m_traded_volume.end(); ++pit)
    {
        if (pit->second > 0.0)
        {
            const double price = pit->first;
            auto expriceit = m_traded_volume.find(price);
            if (expriceit != m_traded_volume.end())
            {
                // The new price is also present in OUR map of prices - update the volume!
                expriceit->second += pit->second;
            }
            else
            {
                // new price not present in OUR map. insert it
                m_traded_volume.insert(std::make_pair(pit->first,pit->second));
            }
        }
    }
    candle_volume = 0.0;
    double wsum = 0.0;
    if (m_traded_volume.empty() == false)
    {
        for (auto pit = m_traded_volume.begin(); pit != m_traded_volume.end(); ++pit)
        {
            const double p = pit->first;
            const double v = pit->second;
            if (p > 0.0 && v > 0.0)
            {
                candle_volume += v;
                wsum += (p * v);
                if (p < low)
                {
                    low = p;
                }
                if (p > high)
                {
                    high = p;
                }
            }
        }
    }
    if (candle_volume > 0.0)
    {
        candle_vwap = wsum/candle_volume;
        if (low == high)
        {
            candle_vwap_as_tick = calculateTickFractionFromOdds(low,low);
        }
        else
        {
            candle_vwap_as_tick = calculateTickFractionFromOdds(low,candle_vwap);
        }
    }
    price_anomalies.insert(price_anomalies.end(),candle.price_anomalies.begin(),candle.price_anomalies.end());

}

}   // end of namespace betfair::utils
}   // end of namespace betfair
