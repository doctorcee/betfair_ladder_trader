#ifndef BET_H
#define BET_H

#include <QString>

namespace betfair {

class TBet
{
public:
    TBet(const QString& mid,
           const QString& bid,
           const QString& persistence,
           const QString& placed_date,
           const QString& placement_status,
           const QString& match_status,
           const QString& err_code,
           const std::int64_t& sid,
           const double& odds,
           const double& stake,
           const double& matched,
           const double& unmatched,
           const double& avg_odds,
           bool b_lay_type);
    QString getDisplayString() const;    
    QString getPlacementStatus() const {return m_placement_status;}
    QString getMatchStatus() const {return m_matched_status;}
    double getStake() const {return m_stake;}
    double getAskPrice() const {return m_ask_price;}
    double getMatchedPortion() const {return m_total_matched;}
    double getUnmatchedPortion() const {return m_total_unmatched;}
    double getLapsedVolume() const {return m_lapsed_volume;}
    double getVoidedVolume() const {return m_voided_volume;}
    double getCancelledVolume() const {return m_cancelled_volume;}
    std::int64_t getSelectionID() const {return m_selection_id;}
    QString getBetID() const {return m_bet_id;}
    bool isLay() const {return m_bet_type_lay;}
    double getAvgMatchOdds() const {return m_avg_matched_odds;}


    QString getSelectionName() const {return m_selection_name;}
    QString getError() const {return m_bet_error_code;}

    void updateSelectionName(const QString& sel_name);
    void updateMatchedPortion(const double& vol);
    void updateUnmatchedPortion(const double& vol);
    void updateAveragePriceMatched(const double& price);

    bool isBetFullyMatched() const {return m_b_executed;}
    bool isVoidLapsedCancelled() const;
    bool isVoided() const {return m_voided_volume > 0.0;}
    bool isCancelled() const {return !m_cancellation_date.isEmpty();}
    bool isLapsed() const {return m_lapsed_volume > 0.0;}

    void updateCancelledVolume(const double& vol);
    void updateVoidedVolume(const double& vol);
    void updateLapsedVolume(const double& vol);
    void updateMatchedStatus(const QString& new_match_status);
    void setCancellationDate(const QString& date);

    void flagAsElapsed() {m_b_flagged_as_lapsed = true;}
    bool flaggedAsElapsed() const {return m_b_flagged_as_lapsed;}

    QString getBetInfoDump() const;

private:
    // No copy, no default constructor, no copy assignment
    TBet();
    TBet(const TBet& cp);
    TBet& operator=(const TBet& cp);

    const QString m_market_id;
    const QString m_bet_id;
    const QString m_persistence_type;
    const QString m_placed_date;
    QString m_placement_status;
    QString m_matched_status;
    QString m_bet_error_code;

    const std::int64_t m_selection_id;
    QString m_selection_name;
    QString m_cancellation_date;

    const double m_ask_price;
    const double m_stake;
    double m_total_matched;
    double m_total_unmatched;
    double m_avg_matched_odds;
    double m_lapsed_volume;
    double m_voided_volume;
    double m_cancelled_volume;

    const bool m_bet_type_lay;
    bool m_b_executed;          // no remaining unmatched portions
    bool m_b_flagged_as_lapsed;

};




} // end of namespace betfair

#endif // BET_H
