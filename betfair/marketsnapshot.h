#ifndef MARKETSNAPSHOT_H
#define MARKETSNAPSHOT_H

#include <QString>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>

namespace betfair {

// class to contain market data returned from API-NG call to listMarketBook

class TMarketSnapshot
{
public:

    TMarketSnapshot(const QString& mkid,
                    const std::int64_t& runner_id,
                    const double timestamp,
                    int bet_delay,
                    const double& lpm,
                    const double& adj,
                    const double& total_vol,
                    const QString& stat,
                    const QString& market_stat,
                    const double last_mk_match_ts);

    void setBackOrders(const std::map<double,double>& data) {m_back_orders = data;}
    void setLayOrders(const std::map<double,double>& data) {m_lay_orders = data;}
    void setTradedHistory(const std::map<double,double>& data) {m_traded_history = data;}

    double getLPM() const {return m_last_price_matched;}
    double getAdjustmentFactor() const {return m_adj_factor;}
    double getTotalVolumeMatched() const {return m_total_matched_volume;}
    double getAverageMatchedPrice() const;
    double getMinMatchedPrice() const;
    double getMaxMatchedPrice() const;

    std::int64_t getSelectionID() const {return m_selection_id;}

    QString getMarketName() const {return m_market_name;}
    QString getMarketID() const {return m_market_id;}
    QString getSelectionName() const {return m_selection_name;}
    QString getStatus() const {return m_status;}
    QString getMarketStatus() const {return m_market_status;}
    double getTimestamp() const {return m_timestamp;}
    double getLastMarketMatchedBetTimestamp() const {return m_last_mkt_matchtime;}

    int getBetDelay() const {return m_bet_delay;}

    const std::map<double,double>& getBackOrders() const {return m_back_orders;}
    const std::map<double,double>& getLayOrders() const {return m_lay_orders;}
    const std::map<double,double>& getTradeHistory() const {return m_traded_history;}

private:
    TMarketSnapshot();
    TMarketSnapshot(const TMarketSnapshot& cp);
    TMarketSnapshot& operator=(const TMarketSnapshot& cp);

    QString m_market_id;
    QString m_market_name;
    QString m_selection_name;
    double m_timestamp;
    QString m_status;
    QString m_market_status;
    std::int64_t m_selection_id;
    int m_bet_delay;

    double m_last_price_matched;
    double m_total_matched_volume;
    double m_adj_factor;
    double m_last_mkt_matchtime;

    std::map<double,double> m_back_orders;
    std::map<double,double> m_lay_orders;
    std::map<double,double> m_traded_history;
};

} // end of namespace betfair

#endif // MARKETSNAPSHOT_H
