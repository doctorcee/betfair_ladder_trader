#ifndef RUNNER_H
#define RUNNER_H

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include <QDateTime>
#include "marketsnapshot.h"
#include "betfairadvancedcandle.h"
#include "runnermetainfo.h"
#include "betfair_utils.h"


namespace betfair {

class TRunner
{
private:
    TRunner();
    TRunner(const TRunner& cp);
    TRunner& operator=(const TRunner& cp);

    TRunnerMetaInfo m_info;
    std::shared_ptr<TMarketSnapshot> m_current_snapshot;
    std::vector<betfair::utils::TBetfairAdvancedCandle> m_adv_candle_data;

    double m_y_min;
    double m_y_max;

    double m_profit;

    double m_higest_traded_price_vol;

    int m_lpm_move_state;

public:
    TRunner(const TRunnerMetaInfo& info);

    std::int64_t getID() const {return m_info.id;}
    int getSortPriority() const {return m_info.sort_priority;}
    QString getName() const {return m_info.name;}
    QString getStallDraw() const {return m_info.stall_draw;}
    QString getJockey() const {return m_info.jockey;}
    double getYMax() const {return m_y_max;}
    double getYMin() const {return m_y_min;}
    QString getSilksURL() const {return m_info.colours_filename;}

    double getDataStartTime() const;
    double getDataEndTime() const;

    double getCurrentAvailableLayVolume(int N) const;
    double getCurrentAvailableBackVolume(int N) const;

    void updateProfitIfWins(const std::int64_t& id, const double& profit);
    std::map<double,double> getAvailableBackPrices() const;
    std::map<double,double> getAvailableLayPrices() const;

    // index passed to these functions works as index FROM BEST PRICE. So for lay
    // prices, this index is taken from the END of the map
    std::pair<double,double> getOrderedBackPrice(int index) const;
    std::pair<double,double> getOrderedLayPrice(int index) const;

    bool update(std::shared_ptr<TMarketSnapshot> new_snapshot,
                QString& msg);

    QString getStatus() const;
    double getReductionFactor() const;
    double getTotalMatched() const;
    double getLPM() const;
    double getProfit() const {return m_profit;}

    bool isActive() const;
    bool isWinner() const;

    int getLPMDirection() const {return m_lpm_move_state;}

    double getAvailableToBackAtOdds(const double odds);
    double getAvailableToLayAtOdds(const double odds);
    double getTotalTraderVolumeAtOdds(const double odds);
    double getTotalTraderVolumeAtOdds(const double odds, const std::uint16_t last_x_seconds);

    double getHighestOddsVolumeTraded() const;

    const std::vector<betfair::utils::TBetfairAdvancedCandle>& getAdvCandleData() const {return m_adv_candle_data;}

    void getLatestIntervalData(double& min, double& max, double& lpm);

};

} // end of namespace betfair


#endif // RUNNER_H
