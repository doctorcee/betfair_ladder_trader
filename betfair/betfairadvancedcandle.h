#ifndef BETFAIRADVANCEDCANDLE_H
#define BETFAIRADVANCEDCANDLE_H

#include <memory>
#include <vector>
#include <map>
#include <QString>
#include "betfair/marketsnapshot.h"

namespace betfair {
namespace utils {


class TBetfairAdvancedCandle {
private:
    std::int64_t selection_id;
    double cumulative_vwap;
    double open;
    double close;
    double high;
    double low;
    double candle_vwap;
    double candle_volume;
    double lpm_as_tick;
    double cumul_vwap_as_tick;
    double candle_vwap_as_tick;
    std::map<double,double> m_traded_volume;
    void calculateCandleRangeAndMean(const double& candle_min_vol_threshold,
                                       double& low,
                                       double& high,
                                       double& volume,
                                       double& weighted_avg_match) const;
    double calculateTickFractionFromOdds(const double& low,
                                         const double& vwap);
public:
    TBetfairAdvancedCandle(const std::int64_t& id);

    TBetfairAdvancedCandle(const std::int64_t& selection_id,
                           const std::shared_ptr<betfair::TMarketSnapshot>& a,
                           const std::shared_ptr<betfair::TMarketSnapshot>& b);

    double global_max_traded;
    double global_min_traded;
    double cumulative_runner_match_total;

    double best_lay_price_available;
    double best_back_price_available;

    std::int64_t market_version;
    double start_timestamp;
    double end_timestamp;
    QString selection_status;
    QString market_status;
    QString market_id;
    bool market_in_play_at_start;
    bool market_in_play_at_end;
    bool market_cross_matched;
    std::vector<std::string> price_anomalies;
    const std::map<double,double>& getTradedVolume() const {return m_traded_volume;}


    void merge(const TBetfairAdvancedCandle& candle);
    std::int64_t getSelectionID() const {return selection_id;}
    double getOpen() const {return open;}
    double getClose() const {return close;}
    double getCumulativeVWAP() const {return cumulative_vwap;}
    double getLow() const {return low;}
    double getHigh() const {return high;}
    double getCandleVWAP() const {return candle_vwap;}
    double getCandleVol() const {return candle_volume;}
    double getCumulativeVWAPAsTick() const {return cumul_vwap_as_tick;}
    double getCandleVWAPAsTick() const {return candle_vwap_as_tick;}
    double getLPMAsTick() const {return lpm_as_tick;}


};

} // end of namespace betfair::utils
} // end of namespace betfair


#endif // BETFAIRADVANCEDCANDLE_H
