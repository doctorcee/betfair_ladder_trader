#ifndef BETFAIRMARKET_H
#define BETFAIRMARKET_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>
#include <memory>
#include "marketinfo.h"
#include "betfair_utils.h"
#include "runner.h"
#include "bet.h"

namespace betfair {

class TBetfairMarket
{
private:
    TBetfairMarket(const TBetfairMarket& cp);
    TBetfairMarket& operator=(const TBetfairMarket& cp);
    bool m_valid;
    bool m_runners_sorted;
    TMarketInfo market_info;

    QString m_market_status;
    int m_bet_delay;
    bool m_b_inplay;
    int m_reported_active_runner_count;
    double m_last_traded_match_timestamp;

    std::int64_t market_version;
    bool m_cross_matched;

    std::vector<std::shared_ptr<TRunner> > m_runners;
    std::vector<std::shared_ptr<TBet> > m_placed_bets;
    std::vector<std::shared_ptr<TBet> > m_fully_matched_bets;
    std::vector<std::shared_ptr<TBet> > m_failed_bets;

    std::map<double,double> m_total_market_volume_data;

public:
    TBetfairMarket();
    QString getMarketID() const {return market_info.getMarketID();}
    QString getMarketName() const {return market_info.getMarketName();}
    QString getMarketDetails() const {return market_info.getMarketDetails();}
    QString getEventName() const {return market_info.getEventName();}
    QDateTime getStartTime() const {return market_info.getStartTime();}
    QString getVenue() const {return market_info.getVenue();}
    QString getMarketStatus() const {return m_market_status;}
    QString getLabel() const {return market_info.getLabel();}
    std::size_t getNumRunners() const {return m_runners.size();}
    int getReportedActiveRunnerCount() const {return m_reported_active_runner_count;}

    double getLastMatchedTimestamp() const {return m_last_traded_match_timestamp;}
    std::int64_t getMarketVersion() const {return market_version;}
    bool crossMatchingEnabled() const {return m_cross_matched;}
    bool marketInPlay() const {return m_b_inplay;}
    int getBetDelay() const {return m_bet_delay;}

    void clear();
    void sortRunners();
    void sortRunnersByTotalMatchedVolume();

    void calculateRunnerExposure(const std::int64_t& sel_id, double& win_pft, double& loss_pft);
    void calculateRunnerExposure(const std::shared_ptr<TRunner>& sel,double& win_pft,double& loss_pft);
    bool valid() const {return m_valid;}

    bool selectionExists(std::int64_t id);
    bool updateRunnerSnapshot(std::int64_t runner_id,
                              const std::shared_ptr<TMarketSnapshot>& new_data,
                              QString& error);
    bool setMarketInfo(const TMarketInfo& info);

    void setMarketStatus(const QString& status) {m_market_status = status;}
    void setBetDelay(const int& bet_delay) {m_bet_delay = bet_delay;}
    void setInPlay(bool ip) {m_b_inplay = ip;}
    void updateLastMatchedTimestamp(const double& new_ts) {m_last_traded_match_timestamp = new_ts;}
    void updateMarketVersion(const std::int64_t& newval) {market_version = newval;};
    void updateCrossMatchedState(bool cm) {m_cross_matched = cm;}

    void updateBetLists();  //  remove matched bets from unmatched list and place in matched list
    void updateRunnerProfitIfWins(const std::int64_t& runnerid,const double& profit);
    void updateReportedActiveRunnerCount(int new_count) {m_reported_active_runner_count = new_count;}

    betfair::utils::betInstruction hedgeRunnerAtAvailableOdds(const std::int64_t& sel,
                                                              const QString& market_id,
                                                              QString& info);

    betfair::utils::betInstruction hedgeRunnerAtFixedOdds(const std::int64_t& sel,
                                                          const QString& market_id,
                                                          const double& odds,
                                                          QString& info);

    double calculateRunnerHedgeProfitAtFixedOdds(const std::int64_t& sel,
                                                 const QString& market_id,
                                                 const double& odds);

    std::vector<QString> getSelectionUnmatchedBetIDs(const std::int64_t& sel,
                                                     const QString& market_id,
                                                     bool laytype);

    std::vector<QString> getSelectionUnmatchedBetByAskPriceIDs(const std::int64_t& sel,
                                                               const QString& market_id,
                                                               const double& ask_price,
                                                               bool laytype);

    void getSelectionWOM(const std::int64_t& sel_id,
                         double& back,
                         double& lay,
                         int N);
    double getSelectionProfitIfWins(const std::int64_t& sel_id);
    double getTotalTradedVolume(const std::int64_t& sel_id);

    bool addPlacedBet(const std::shared_ptr<TBet>& new_bet, QString& info);
    const std::vector<std::shared_ptr<TBet> >& getPlacedBets() const;
    const std::vector<std::shared_ptr<TBet> >& getFailedBets() const;
    const std::vector<std::shared_ptr<TBet> >& getFullyMatchedBets() const;
    void clearFailedBets();
    std::size_t getNumPlacedBets() const {return m_placed_bets.size();}
    std::size_t getNumMatchedBets() const {return m_fully_matched_bets.size();}
    std::size_t getNumFailedBets() const {return m_failed_bets.size();}

    double getTotalMatchedVolume() const;
    double getRunnerUnmatchedVolAtOdds(const std::int64_t& sel_id,
                                       const double& target_odds,
                                       bool laytype);

    double getBackSideBookPercentage() const ;
    double getLaySideBookPercentage() const ;

    void updateMarketTotalMatched(const double& ts, const double& total);
    const std::map<double,double>& getTotalMarketVolumeSeries() const {return m_total_market_volume_data;}

    const TMarketInfo& getMarketInfo() const {return market_info;}
    std::vector<std::int64_t> getSelectionIDList() const;
    const std::vector<std::shared_ptr<TRunner> >& getRunners() const {return m_runners;}
    std::shared_ptr<TRunner> getSelectionByID(std::int64_t id);
    std::shared_ptr<TRunner> getSelectionByName(const QString& name);
    std::shared_ptr<TBet> getBetRecordByBetID(const QString& id);    

    void removeLapsedBets(const std::set<QString>& bets,QString& info);

    bool closed() const {return (m_market_status == "CLOSED");}

    void logMarketBetData();

};

} // end of namespace betfair

#endif // BETFAIRMARKET_H
