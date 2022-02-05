#include "bet.h"
#include <sstream>

namespace betfair {


//=================================================================
TBet::TBet(const QString& mid,
           const QString& bid,
           const QString& persistence,
           const QString& placed_date,
           const QString& status,
           const QString& match_status,
           const QString& err_code,
           const std::int64_t& sid,
           const double& odds,
           const double& stake,
           const double& matched,
           const double& unmatched,
           const double& avg_odds,
           bool b_lay_type)
    : m_market_id(mid),
      m_bet_id(bid),
      m_persistence_type(persistence),
      m_placed_date(placed_date),
      m_placement_status(status),
      m_matched_status(match_status),
      m_bet_error_code(err_code),
      m_selection_id(sid),
      m_selection_name(""),
      m_cancellation_date(""),
      m_ask_price(odds),
      m_stake(stake),
      m_total_matched(matched),
      m_total_unmatched(unmatched),
      m_avg_matched_odds(avg_odds),
      m_lapsed_volume(0.0),
      m_voided_volume(0.0),
      m_cancelled_volume(0.0),
      m_bet_type_lay(b_lay_type),
      m_b_executed(m_matched_status == "EXECUTION_COMPLETE"),
      m_b_flagged_as_lapsed(false)

{
}

//=================================================================
void TBet::updateSelectionName(const QString& sel_name)
{
    if (m_selection_name.isEmpty())
    {
        m_selection_name = sel_name;
    }
}

//=================================================================
void TBet::updateMatchedPortion(const double& vol)
{
    m_total_matched = vol;
}

//=================================================================
void TBet::updateUnmatchedPortion(const double& vol)
{
    m_total_unmatched = vol;
}

//=================================================================
void TBet::setCancellationDate(const QString& date)
{
    if (m_cancellation_date.isEmpty() && !isBetFullyMatched())
    {
        m_cancellation_date = date;
    }
}

//=================================================================
void TBet::updateAveragePriceMatched(const double& price)
{
    m_avg_matched_odds = price;
}

//=================================================================
void TBet::updateCancelledVolume(const double& vol)
{
    m_cancelled_volume = vol;
}

//=================================================================
void TBet::updateVoidedVolume(const double& vol)
{
    m_voided_volume = vol;
}

//=================================================================
void TBet::updateLapsedVolume(const double& vol)
{
    m_lapsed_volume = vol;
}

//=================================================================
bool TBet::isVoidLapsedCancelled() const
{
    return (m_lapsed_volume > 0.0) || (m_voided_volume > 0.0) || (m_cancelled_volume > 0.0);
}

//=================================================================
void TBet::updateMatchedStatus(const QString& new_status)
{
    m_matched_status = new_status;
    if (m_matched_status == "EXECUTION_COMPLETE")
    {
        m_b_executed = true;
    }
}

//=================================================================
QString TBet::getBetInfoDump() const
{
    // Generate a full information dump for this bet
    std::ostringstream infostr;
    infostr.precision(2);
    infostr << "Bet ID: " <<  m_bet_id.toStdString() << std::endl;
    if (m_bet_type_lay)
    {
        infostr << "Bet Type: LAY" << std::endl;
    }
    else
    {
        infostr << "Bet Type: BACK" << std::endl;
    }
    infostr << "Market ID: " << m_market_id.toStdString() << std::endl;
    infostr << "Selection ID: " << m_selection_id << " [" << m_selection_name.toStdString() + "]" << std::endl;
    infostr << "Stake: £" << std::fixed << m_stake << std::endl;
    infostr << "Ask Price: " << std::fixed << m_ask_price << std::endl;
    infostr << "Placed Date: " << m_placed_date.toStdString() << std::endl;
    infostr << "Cancelled Date: " << m_cancellation_date.toStdString() << std::endl;
    infostr << "Matched Status: " << m_matched_status.toStdString() << std::endl;
    infostr << "Error Code: " << m_bet_error_code.toStdString() << std::endl;
    infostr << "Persistence Type: " << m_persistence_type.toStdString() << std::endl;
    infostr << "Placement Status: " << m_placement_status.toStdString() << std::endl;
    infostr << "Matched: £" << std::fixed << m_total_matched << std::endl;
    infostr << "Unmatched: £" << std::fixed << m_total_unmatched << std::endl;
    infostr << "Average Odds: £" << std::fixed << m_avg_matched_odds << std::endl;
    infostr << "Lapsed: £" << std::fixed << m_lapsed_volume << std::endl;
    infostr << "Voided: £" << std::fixed << m_voided_volume << std::endl;
    infostr << "Cancelled: £" << std::fixed << m_cancelled_volume << std::endl;

    return QString::fromStdString(infostr.str());
}

//=================================================================
QString TBet::getDisplayString() const
{
    // Function to return info string about bet details for GUI
    std::ostringstream ost;
    ost.precision(2);
    double liability = m_stake;
    if (m_bet_type_lay)
    {
        liability = (m_ask_price - 1.0)*m_stake;
        ost << "LAY ";
    }
    else
    {
        ost << "BACK ";
    }
    ost << m_selection_name.toStdString() << " £" << std::fixed << m_stake << " @ " << m_ask_price;
    ost << " [Matched: £" << m_total_matched << " @ " << m_avg_matched_odds << "]";
    return QString::fromStdString(ost.str());
}

} // end of namespace betfair
