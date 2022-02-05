#include "betfairmarket.h"
#include <sstream>


namespace betfair {

// functor for finding runners by ID
struct runnerIDComparison {
    explicit runnerIDComparison(const std::int64_t& id) : my_id(id) { }
    inline bool operator()(const std::shared_ptr<TRunner>& sel) const { return (sel && (my_id == sel->getID()));}
private:
    std::int64_t my_id;
};

struct runnerNameComparison {
    explicit runnerNameComparison(const QString& name) : my_name(name) { }
    inline bool operator()(const std::shared_ptr<TRunner>& sel) const { return (sel && (my_name == sel->getName()));}
private:
    QString my_name;
};

// functors for sorting vector of std::shared_ptr<TRunner>
class sortByRFAsc {
public:
    sortByRFAsc(bool b_asc) : m_asc(b_asc) {}
    bool operator()(std::shared_ptr<TRunner> const &a, std::shared_ptr<TRunner> const &b) {
        bool b_ret = m_asc ? (a->getReductionFactor() < b->getReductionFactor()) : (a->getReductionFactor() > b->getReductionFactor());
        if (!a->isActive())
        {
            b_ret = m_asc ? true : false;
        }
        if (!b->isActive())
        {
            b_ret = m_asc ? false : true;
        }
        return b_ret;
    }
private:
    bool m_asc;

};

class sortBySelID {
public:
    bool operator()(std::shared_ptr<TRunner> const &a, std::shared_ptr<TRunner> const &b) {
        return (a->getID() < b->getID());
    }
};

class sortByLPMAsc {
public:
    sortByLPMAsc(bool b_asc) : m_asc(b_asc) {}
    bool operator()(std::shared_ptr<TRunner> const &a, std::shared_ptr<TRunner> const &b) {
        return m_asc ? (a->getLPM() < b->getLPM()) : (a->getLPM() > b->getLPM());
    }
private:
    bool m_asc;

};

struct betIDComparison {
    explicit betIDComparison(const QString& id) : my_id(id) { }
    inline bool operator()(const std::shared_ptr<betfair::TBet>& bet) const { return (bet && (my_id == bet->getBetID()));}
private:
    QString my_id;
};

bool isBetMatched(const std::shared_ptr<TBet>& bet)
{
    return (bet && bet->isBetFullyMatched());
}

bool isBetCancelled(const std::shared_ptr<TBet>& bet)
{
    return (bet && bet->isCancelled());
}

bool isBetFinished(const std::shared_ptr<TBet>& bet)
{
    return (bet && (bet->getUnmatchedPortion() < 0.001));
}

bool isBetFlaggedAsLapsed(const std::shared_ptr<TBet>& bet)
{
    return (bet && bet->flaggedAsElapsed());
}

//====================================================================
TBetfairMarket::TBetfairMarket()
    : m_valid(false),
      m_runners_sorted(false),
      m_market_status("UNKNOWN"),
      m_bet_delay(-1),
      m_b_inplay(false),
      m_reported_active_runner_count(-1),
      m_last_traded_match_timestamp(0.0),
      market_version(0),
      m_cross_matched(false)
{
}

//====================================================================
void TBetfairMarket::clear()
{
    m_valid = false;
    m_runners_sorted = false;
    market_info.reset();
    m_market_status = "UNKNOWN";
    m_b_inplay = false;
    m_bet_delay = -1;
    m_reported_active_runner_count = -1;
    m_last_traded_match_timestamp = 0.0;
    market_version = 0;
    m_runners.clear();
    m_placed_bets.clear();
    m_fully_matched_bets.clear();
    m_failed_bets.clear();
    m_total_market_volume_data.clear();
}

//====================================================================
bool TBetfairMarket::setMarketInfo(const TMarketInfo& info)
{
    clear();
    m_valid = false;
    if (!info.getMarketID().isEmpty())
    {
        bool errors = false;
        // Set new info object and populate runners array
        market_info = info;
        const std::map<std::int64_t, TRunnerMetaInfo>& runners = market_info.getAllMetaData();
        if (!runners.empty())
        {
            for (auto it = runners.begin(); it!= runners.end(); ++it)
            {
                std::shared_ptr<TRunner> new_runner = std::make_shared<TRunner>(it->second);
                // Search for THIS runner in the vector already
                if (m_runners.end() == std::find_if(m_runners.begin(),m_runners.end(),runnerIDComparison(it->first)))
                {
                    // Good - this is not in the vector already
                    m_runners.push_back(new_runner);
                }
                else
                {
                    errors = true;
                    break;
                }
            }
        }
        else
        {
            errors = true;
        }
        m_valid = !errors;
    }
    if (m_valid)
    {
        // Sort runners by RF

    }
    else
    {
        m_runners.clear();
    }
    return m_valid;
}

//====================================================================
bool TBetfairMarket::selectionExists(std::int64_t id)
{
    return (std::find_if(m_runners.begin(), m_runners.end(), runnerIDComparison(id)) != m_runners.end());
}

//====================================================================
std::shared_ptr<TRunner> TBetfairMarket::getSelectionByID(std::int64_t id)
{
    std::shared_ptr<TRunner> ret_val;
    auto it = std::find_if(m_runners.begin(),m_runners.end(),runnerIDComparison(id));
    if (it != m_runners.end())
    {
        ret_val = *it;
    }
    return ret_val;
}

//====================================================================
std::shared_ptr<TRunner> TBetfairMarket::getSelectionByName(const QString& name)
{
    std::shared_ptr<TRunner> ret_val;
    auto it = std::find_if(m_runners.begin(),m_runners.end(),runnerNameComparison(name));
    if (it != m_runners.end())
    {
        ret_val = *it;
    }
    return ret_val;
}

//====================================================================
void TBetfairMarket::sortRunnersByTotalMatchedVolume()
{
    if (!m_runners_sorted)
    {
        // Use a lambda to sort by matched volume (descending)
        std::sort(m_runners.begin(), m_runners.end(),
            [](const std::shared_ptr<betfair::TRunner>& a, std::shared_ptr<betfair::TRunner>& b) -> bool
        {
            return a->getTotalMatched() > b->getTotalMatched();
        });
        m_runners_sorted = true;
    }
}

//====================================================================
void TBetfairMarket::sortRunners()
{
    if (!m_runners_sorted)
    {
        // Sort by traded volume for HORSE racing. We dont want dogs sorted
        // as we want to retain trap number order. Similarly for football
        // we want home team first
        if (market_info.getEventTypeID() == "7")
        {            
            sortRunnersByTotalMatchedVolume();            
        }
        else
        {
            // Not horse racing - use a lambda to sort by sort priority ascending
            std::sort(m_runners.begin(), m_runners.end(),
                [](const std::shared_ptr<betfair::TRunner>& a, std::shared_ptr<betfair::TRunner>& b) -> bool
            {
                return a->getSortPriority() < b->getSortPriority();
            });
        }
        m_runners_sorted = true;
    }
}


//====================================================================
std::vector<std::int64_t> TBetfairMarket::getSelectionIDList() const
{
    std::vector<std::int64_t> v_list;
    for (auto it = m_runners.begin(); it != m_runners.end(); ++it)
    {
        v_list.push_back((*it)->getID());
    }
    return v_list;
}

//====================================================================
bool TBetfairMarket::updateRunnerSnapshot(std::int64_t runner_id,
                                          const std::shared_ptr<TMarketSnapshot>& new_data,
                                          QString& error)
{
    // Create new candle from last input snapshot and this new one
    bool success = false;
    error = "";
    if (new_data)
    {
        if (new_data->getSelectionID() == runner_id)
        {
            auto it = std::find_if(m_runners.begin(),m_runners.end(),runnerIDComparison(runner_id));
            if (it != m_runners.end())
            {
                success = (*it)->update(new_data,error);
            }
        }
    }
    return success;
}

//====================================================================
const std::vector<std::shared_ptr<TBet> >& TBetfairMarket::getPlacedBets() const
{
    return m_placed_bets;
}

//====================================================================
const std::vector<std::shared_ptr<TBet> >& TBetfairMarket::getFailedBets() const
{
    return m_failed_bets;
}

//====================================================================
const std::vector<std::shared_ptr<TBet> >& TBetfairMarket::getFullyMatchedBets() const
{
    return m_fully_matched_bets;
}


//====================================================================
void TBetfairMarket::clearFailedBets()
{
    m_failed_bets.clear();
}

//====================================================================
void TBetfairMarket::getSelectionWOM(const std::int64_t& sel_id,
                                     double& back,
                                     double& lay,
                                     int N)
{
    auto selection = this->getSelectionByID(sel_id);
    back = 0.0;
    lay = 0.0;
    if (selection && selection->isActive() && (N > 0))
    {
        lay = selection->getCurrentAvailableLayVolume(N);
        back = selection->getCurrentAvailableBackVolume(N);
    }
}

//====================================================================
double TBetfairMarket::getSelectionProfitIfWins(const std::int64_t& sel_id)
{
    auto selection = getSelectionByID(sel_id);
    double profit = 0.0;
    if (selection && selection->isActive())
    {
        profit = selection->getProfit();
    }
    return profit;
}

//====================================================================
double TBetfairMarket::getTotalTradedVolume(const std::int64_t& sel_id)
{
    auto selection = getSelectionByID(sel_id);
    double vol = 0.0;
    if (selection && selection->isActive())
    {
        vol = selection->getTotalMatched();
    }
    return vol;
}

//====================================================================
betfair::utils::betInstruction TBetfairMarket::hedgeRunnerAtFixedOdds(const std::int64_t& sel,
                                                                      const QString& market_id,
                                                                      const double& odds,
                                                                      QString& info)
{
    betfair::utils::betInstruction bet;
    bet.market_id = "";
    info = "";
    bet.selection = 0;
    bet.b_lay_type = false;
    bet.odds = 0.0;
    bet.stake = 0.0;
    if (market_id == market_info.getMarketID() && odds > 1.00)
    {
        auto selection = this->getSelectionByID(sel);
        if (selection)
        {
            if (selection->isActive())
            {
                QString runner_name = selection->getName();
                bet.market_id = market_id;
                bet.selection = sel;
                double win_pft = 0.0;
                double loss_pft = 0.0;
                calculateRunnerExposure(selection,win_pft,loss_pft);
                info = "Market exposure for " + runner_name + ": If WINS = £" + QString::number(win_pft,'f',2) + ", if LOSES = £" + QString::number(loss_pft,'f',2);
                if (loss_pft < win_pft)
                {
                    // lay
                    double pft_diff = win_pft - loss_pft;
                    double d_stake = pft_diff/odds;
                    if (d_stake >= betfair::utils::min_betting_stake)
                    {
                        bet.stake = d_stake;
                        bet.b_lay_type = true;
                        bet.odds = odds;
                        info += "TBetfairMarket::hedgeRunnerAtFixedOdds() : HEDGING position on runner " + runner_name + ": LAYING @ " + QString::number(odds,'f',2) + " for £" + QString::number(d_stake,'f',2);
                    }
                    else
                    {
                        info += "TBetfairMarket::hedgeRunnerAtFixedOdds() error : HEDGING position on runner " + runner_name + " unsuccessful - action required utilises lower than minimum allowed stakes";
                    }
                }
                else if (loss_pft > win_pft)
                {
                    // back
                    double pft_diff = loss_pft - win_pft;
                    double d_stake = pft_diff/odds;
                    if (d_stake >= betfair::utils::min_betting_stake)
                    {
                        bet.stake = d_stake;
                        bet.b_lay_type = false;
                        bet.odds = odds;
                        info += "TBetfairMarket::hedgeRunnerAtFixedOdds() : HEDGING position on runner " + runner_name + ": BACKING @ " + QString::number(odds,'f',2) + " for £" + QString::number(d_stake,'f',2);
                    }
                    else
                    {
                        info += "TBetfairMarket::hedgeRunnerAtFixedOdds() : HEDGING position on runner " + runner_name + " unsuccessful - action required utilises lower than minimum allowed stakes";
                    }
                }
                else
                {
                    info += "TBetfairMarket::hedgeRunnerAtFixedOdds() : No hedge available on " + runner_name + " - profit and loss already match!";
                }
            }
            else
            {
                info = "TBetfairMarket::hedgeRunnerAtFixedOdds() : Selection not an active runner!";
            }
        }
        else
        {
            info = "TBetfairMarket::hedgeRunnerAtFixedOdds() : Selection not found!";
        }
    }
    else
    {
        info = "TBetfairMarket::hedgeRunnerAtFixedOdds() : Market ID mismatch!";
    }
    return bet;
}

//====================================================================
double TBetfairMarket::calculateRunnerHedgeProfitAtFixedOdds(const std::int64_t& sel,
                                                             const QString& market_id,
                                                             const double& odds)
{
    double pft = 0.0;
    if (market_id == market_info.getMarketID())
    {
        auto selection = this->getSelectionByID(sel);
        if (selection)
        {
            if (selection->isActive())
            {
                double win_pft = 0.0;
                double loss_pft = 0.0;
                calculateRunnerExposure(selection,win_pft,loss_pft);
                if (loss_pft < win_pft)
                {
                    // need to lay                   
                    double pft_diff = win_pft - loss_pft;
                    pft = (pft_diff/odds) + loss_pft;
                }
                else if (loss_pft > win_pft)
                {
                    // need to back                    
                    double pft_diff = loss_pft - win_pft;
                    double stake = pft_diff/odds;                    
                    pft = win_pft + (stake*(odds-1.0));                    
                }
                else
                {
                    pft = loss_pft;
                }
            }
        }
    }
    return pft;
}

//====================================================================
betfair::utils::betInstruction TBetfairMarket::hedgeRunnerAtAvailableOdds(const std::int64_t& sel,
                                                                          const QString& market_id,
                                                                          QString& info)
{
    betfair::utils::betInstruction bet;
    bet.market_id = "";
    info = "";
    bet.selection = 0;
    bet.b_lay_type = false;
    bet.odds = 0.0;
    bet.stake = 0.0;
    if (market_id == market_info.getMarketID())
    {
        auto selection = this->getSelectionByID(sel);
        if (selection)
        {
            if (selection->isActive())
            {
                QString runner_name = selection->getName();
                bet.market_id = market_id;
                bet.selection = sel;
                double win_pft = 0.0;
                double loss_pft = 0.0;
                calculateRunnerExposure(selection,win_pft,loss_pft);
                info = "Market exposure for " + runner_name + ": If WINS = £" + QString::number(win_pft,'f',2) + ", if LOSES = £" + QString::number(loss_pft,'f',2);               
                if (loss_pft < win_pft)
                {
                    // need to lay
                    std::pair<double,double> lp = selection->getOrderedLayPrice(0);
                    const double d_odds = lp.first;
                    if (d_odds > 1.0)
                    {                      
                        double pft_diff = win_pft - loss_pft;
                        double d_stake = pft_diff/d_odds;
                        if (d_stake >= betfair::utils::min_betting_stake)
                        {
                            bet.stake = d_stake;
                            bet.b_lay_type = true;
                            bet.odds = d_odds;
                            info += "TBetfairMarket::hedgeRunnerAtAvailableOdds() : HEDGING position on runner " + runner_name + ": LAYING @ " + QString::number(d_odds,'f',2) + " for £" + QString::number(d_stake,'f',2);
                        }
                        else
                        {
                            info += "TBetfairMarket::hedgeRunnerAtAvailableOdds() error : HEDGING position on runner " + runner_name + " unsuccessful - action required utilises lower than minimum allowed stakes";
                        }
                    }
                }
                else if (loss_pft > win_pft)
                {
                    // need to back
                    std::pair<double,double> bp = selection->getOrderedBackPrice(0);
                    double d_odds = bp.first;
                    if (d_odds > 1.0)
                    {
                        double pft_diff = loss_pft - win_pft;
                        double d_stake = pft_diff/d_odds;
                        if (d_stake >= betfair::utils::min_betting_stake)
                        {
                            bet.stake = d_stake;
                            bet.b_lay_type = false;
                            bet.odds = d_odds;
                            info += "TBetfairMarket::hedgeRunnerAtAvailableOdds() : HEDGING position on runner " + runner_name + ": BACKING @ " + QString::number(d_odds,'f',2) + " for £" + QString::number(d_stake,'f',2);
                        }
                        else
                        {
                            info += "TBetfairMarket::hedgeRunnerAtAvailableOdds() : HEDGING position on runner " + runner_name + " unsuccessful - action required utilises lower than minimum allowed stakes";
                        }
                    }
                }
                else
                {
                    info += "TBetfairMarket::hedgeRunnerAtAvailableOdds() : No hedge available on " + runner_name + " - profit and loss already match!";
                }
            }
            else
            {
                info = "TBetfairMarket::hedgeRunnerAtAvailableOdds() : Selection not an active runner!";
            }
        }
        else
        {
            info = "TBetfairMarket::hedgeRunnerAtAvailableOdds() : Selection not found!";
        }
    }
    else
    {
        info = "TBetfairMarket::hedgeRunnerAtAvailableOdds() : Market ID mismatch!";
    }
    return bet;
}

//====================================================================
void TBetfairMarket::calculateRunnerExposure(const std::shared_ptr<TRunner>& sel,
                                             double& win_pft,
                                             double& loss_pft)
{
    // Examine ALL bets and calculate profits if this runner wins and losses
    // if this runner loses;
    win_pft = 0.0;
    loss_pft = 0.0;
    if (sel)
    {
        QString runnername = sel->getName();
        const std::int64_t sel_id = sel->getID();
        for (auto mbet_it = m_fully_matched_bets.begin(); mbet_it != m_fully_matched_bets.end(); ++mbet_it)
        {
            if ((*mbet_it)->getSelectionID() == sel_id)
            {
                double matched_stake = (*mbet_it)->getMatchedPortion();
                double avg_odds = (*mbet_it)->getAvgMatchOdds();
                if ((*mbet_it)->isLay())
                {
                    // LAY bet
                    win_pft -= (matched_stake * (avg_odds-1.0));
                    loss_pft += matched_stake;
                }
                else
                {
                    // BACK bet
                    win_pft += (matched_stake * (avg_odds-1.0));
                    loss_pft -= matched_stake;
                }
            }
        }
        // Now do the same for partially matched bets
        for (auto mbet_it = m_placed_bets.begin(); mbet_it != m_placed_bets.end(); ++mbet_it)
        {
            if ((*mbet_it)->getSelectionID() == sel_id)
            {
                double matched_stake = (*mbet_it)->getMatchedPortion();
                if (matched_stake > 0.0)
                {
                    double avg_odds = (*mbet_it)->getAvgMatchOdds();
                    if ((*mbet_it)->isLay())
                    {
                        // LAY bet
                        win_pft -= (matched_stake * (avg_odds-1.0));
                        loss_pft += matched_stake;
                    }
                    else
                    {
                        // BACK bet
                        win_pft += (matched_stake * (avg_odds-1.0));
                        loss_pft -= matched_stake;
                    }
                }
            }
        }
    }
}

//====================================================================
double TBetfairMarket::getBackSideBookPercentage() const
{
    double val = 0.0;
    for (auto it = m_runners.begin(); it != m_runners.end(); ++it)
    {
        std::pair<double,double> pricedata = (*it)->getOrderedBackPrice(0);
        if (pricedata.first > 1.0)
        {
            val += 100.0/pricedata.first;
        }
    }
    return val;
}

//====================================================================
double TBetfairMarket::getLaySideBookPercentage() const
{
    double val = 0.0;
    for (auto it = m_runners.begin(); it != m_runners.end(); ++it)
    {
        std::pair<double,double> pricedata = (*it)->getOrderedLayPrice(0);
        if (pricedata.first > 1.0)
        {
            val += 100.0/pricedata.first;
        }
    }
    return val;
}

//====================================================================
void TBetfairMarket::calculateRunnerExposure(const std::int64_t& runner_id,
                                             double& win_pft,
                                             double& loss_pft)
{
    calculateRunnerExposure(getSelectionByID(runner_id),win_pft,loss_pft);
}

//====================================================================
std::shared_ptr<TBet> TBetfairMarket::getBetRecordByBetID(const QString& id)
{
    std::shared_ptr<TBet> ret_val;
    bool found = false;
    auto it = std::find_if(m_placed_bets.begin(),m_placed_bets.end(),betIDComparison(id));
    if (it != m_placed_bets.end())
    {
        found = true;
        ret_val = *it;
    }
    if (false == found)
    {
        // look in matched bets too
        auto it = std::find_if(m_fully_matched_bets.begin(),m_fully_matched_bets.end(),betIDComparison(id));
        if (it != m_fully_matched_bets.end())
        {
            found = true;
            ret_val = *it;
        }
    }
    return ret_val;
}

//====================================================================
void TBetfairMarket::updateRunnerProfitIfWins(const std::int64_t& id,
                                              const double& profit)
{
    auto it = std::find_if(m_runners.begin(),m_runners.end(),runnerIDComparison(id));
    if (it != m_runners.end())
    {
        std::shared_ptr<TRunner> sel = *it;
        if (sel)
        {
            sel->updateProfitIfWins(id,profit);
        }
    }
}

//====================================================================
void TBetfairMarket::updateMarketTotalMatched(const double& ts,
                                              const double& total)
{
    if (m_total_market_volume_data.empty() == false)
    {
        if ((*m_total_market_volume_data.rbegin()).first < ts)
        {
            m_total_market_volume_data[ts] = total;
        }
    }
    else
    {
        m_total_market_volume_data[ts] = total;
    }
}

//====================================================================
double TBetfairMarket::getTotalMatchedVolume() const
{
    return m_total_market_volume_data.empty() ? 0.0 : m_total_market_volume_data.rbegin()->second;
}

//====================================================================
void TBetfairMarket::logMarketBetData()
{
    // Log all market bet details - we need this to identify the bugs
    // with calculating runner exposure
    const QString logfilename = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_market_" + market_info.getMarketID() + "_logfile.txt";
    std::ostringstream ost;
    ost << "**** Bet Log for Market " << market_info.getMarketID().toStdString() << " " << market_info.getMarketName().toStdString() << " ****" << std::endl;
    ost << "**** Matched bets *****" << std::endl;
    for (auto it = m_fully_matched_bets.begin(); it != m_fully_matched_bets.end(); ++it)
    {
        ost << ((*it)->getBetInfoDump()).toStdString() << std::endl;
    }

    ost << "**** Unmatched / partially matched bets *****" << std::endl;
    for (auto it = m_placed_bets.begin(); it != m_placed_bets.end(); ++it)
    {
        ost << ((*it)->getBetInfoDump()).toStdString() << std::endl;
    }
    betfair::utils::logMessage(logfilename,QString::fromStdString(ost.str()));
}

//====================================================================
void TBetfairMarket::updateBetLists()
{
    const QString logfilename = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_market_" + market_info.getMarketID() + "_logfile.txt";
    std::ostringstream ost;
    for (auto it = m_placed_bets.begin(); it != m_placed_bets.end(); ++it)
    {
        if ((*it)->isBetFullyMatched())
        {
            // log this transition
            m_fully_matched_bets.push_back(*it);
            ost << "[Market ID " + market_info.getMarketID().toStdString() << "] Bet ID " << (*it)->getBetID().toStdString() << " now fully matched!" << std::endl;
            ost << (*it)->getBetInfoDump().toStdString() << std::endl;
            betfair::utils::logMessage(logfilename,QString::fromStdString(ost.str()));
        }

    }
    m_placed_bets.erase(std::remove_if(m_placed_bets.begin(), m_placed_bets.end(), isBetMatched), m_placed_bets.end());
}

//====================================================================
std::vector<QString> TBetfairMarket::getSelectionUnmatchedBetByAskPriceIDs(const std::int64_t& sel,
                                                                           const QString& market_id,
                                                                           const double& ask_price,
                                                                           bool laytype)
{
    std::vector<QString> bets;
    if (market_id == market_info.getMarketID())
    {
        for (auto it = m_placed_bets.begin(); it != m_placed_bets.end(); ++it)
        {
            if ((*it)->getSelectionID() == sel)
            {
                if ((*it)->isLay() == laytype)
                {
                    if ((*it)->getAskPrice() == ask_price)
                    {
                        bets.push_back((*it)->getBetID());
                    }
                }
            }
        }
    }
    return bets;
}

//====================================================================
std::vector<QString> TBetfairMarket::getSelectionUnmatchedBetIDs(const std::int64_t& sel,
                                                                 const QString& market_id,
                                                                 bool laytype)
{
    std::vector<QString> bets;
    if (market_id == market_info.getMarketID())
    {
        for (auto it = m_placed_bets.begin(); it != m_placed_bets.end(); ++it)
        {
            if ((*it)->getSelectionID() == sel)
            {
                if ((*it)->isLay() == laytype)
                {
                    bets.push_back((*it)->getBetID());
                }
            }
        }
    }
    return bets;
}
//====================================================================
double TBetfairMarket::getRunnerUnmatchedVolAtOdds(const std::int64_t& sel_id,
                                                   const double& target_odds,
                                                   bool laytype)
{
    double vol = 0.0;
    for (auto it = m_placed_bets.begin(); it != m_placed_bets.end(); ++it)
    {
        if ((*it)->getSelectionID() == sel_id)
        {
            if ((*it)->isLay() == laytype)
            {
                if (((*it)->getAskPrice() < target_odds + 1e-6) && ((*it)->getAskPrice() > target_odds - 1e-6))
                {
                    vol += (*it)->getUnmatchedPortion();
                }
            }
        }
    }
    return vol;
}

//====================================================================
void TBetfairMarket::removeLapsedBets(const std::set<QString>& bets,
                                      QString& info)
{
    // The input argument should be a list of bets returned by a call to
    // listCurrentOrders. Because lapsed bets will not be returned by this
    // call, we must detect them by bets present in our lists but NOT present
    // in the input set
    const QString logfilename = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_market_" + market_info.getMarketID() + "_logfile.txt";
    info = "";
    for (auto it = m_placed_bets.begin(); it != m_placed_bets.end(); ++it)
    {
        QString betid = (*it)->getBetID();
        bool bet_found = (bets.find(betid) != bets.end());
        if (!bet_found)
        {
            // This bet is not present in the list
            info += "TBetfairmarket::removeLapsedBets Bet ID " + betid + " has been flagged as elapsed and will be removed\n";
            (*it)->flagAsElapsed();
            std::ostringstream betinfo;
            betinfo << "Bet ID " << betid.toStdString() << " has been flagged as lapsed/cancelled as it is missing from listCurrentOrders response and will now be removed" << std::endl;
            betinfo << (*it)->getBetInfoDump().toStdString();
            betfair::utils::logMessage(logfilename,QString::fromStdString(betinfo.str()));
        }
    }
    m_placed_bets.erase(std::remove_if(m_placed_bets.begin(), m_placed_bets.end(), isBetFlaggedAsLapsed), m_placed_bets.end());

    for (auto it = m_fully_matched_bets.begin(); it != m_fully_matched_bets.end(); ++it)
    {
        QString betid = (*it)->getBetID();
        bool bet_found = (bets.find(betid) != bets.end());
        if (!bet_found)
        {
            // Warning - bets in this list should not be removed as they
            // are fully matched.
            info += "TBetfairmarket::removeLapsedBets WARNING - Matched Bet ID " + betid + " is not present in response to listCurrentOrders request!\n";
            std::ostringstream betinfo;
            betinfo << "Bet ID " << betid.toStdString() << " is a MATCHED bet but is missing from listCurrentOrders response!" << std::endl;
            betinfo << (*it)->getBetInfoDump().toStdString();
            betfair::utils::logMessage(logfilename,QString::fromStdString(betinfo.str()));
        }
    }

}

//====================================================================
bool TBetfairMarket::addPlacedBet(const std::shared_ptr<TBet>& new_bet, QString& info)
{
    info = "";
    bool error = false;
    const QString logfilename = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_market_" + market_info.getMarketID() + "_logfile.txt";
    if (new_bet)
    {
        const QString betid = new_bet->getBetID();
        QString sel_name = "UNKNOWN";
        std::shared_ptr<betfair::TRunner> selection = getSelectionByID(new_bet->getSelectionID());
        if (selection)
        {
            sel_name = selection->getName();
        }
        new_bet->updateSelectionName(sel_name);
        QString status = new_bet->getPlacementStatus();
        if (status == "SUCCESS")
        {
            if (new_bet->isBetFullyMatched())
            {
                auto it = std::find_if(m_fully_matched_bets.begin(),m_fully_matched_bets.end(),betIDComparison(betid));
                if (it == m_fully_matched_bets.end())
                {                    
                    m_fully_matched_bets.push_back(new_bet);    
                    QString msg = "[Market ID " + market_info.getMarketID() + "] Bet ID " + new_bet->getBetID() + " fully matched at time of placement.\n";
                    msg += new_bet->getBetInfoDump();
                    betfair::utils::logMessage(logfilename,msg);
                }
                else
                {
                    error = true;
                    info = "TBetfairmarket::addPlacedBet() ERROR - Bet ID " + betid + " is already present in fully matched bet list!";
                    betfair::utils::logMessage(logfilename,info);
                }
            }
            else
            {
                auto it = std::find_if(m_placed_bets.begin(),m_placed_bets.end(),betIDComparison(betid));
                if (it == m_placed_bets.end())
                {
                    m_placed_bets.push_back(new_bet);                    
                    QString msg = "[Market ID " + market_info.getMarketID() + "] Bet ID " + new_bet->getBetID() + " new bet placed (not fully matched at time of placement).\n";
                    msg.append(new_bet->getBetInfoDump());
                    betfair::utils::logMessage(logfilename,msg);
                }
                else
                {
                    error = true;
                    info = "TBetfairmarket::addPlacedBet() ERROR - Bet ID " + betid + " is already present in placed bet list!";
                    betfair::utils::logMessage(logfilename,info);
                }
            }
        }
        else
        {
            auto it = std::find_if(m_failed_bets.begin(),m_failed_bets.end(),betIDComparison(betid));
            if (it == m_failed_bets.end())
            {
                m_failed_bets.push_back(new_bet);                
                QString msg = "[Market ID " + market_info.getMarketID() + "] Bet ID " + new_bet->getBetID() + " - bet placement failed!\n";
                msg.append(new_bet->getBetInfoDump());
                betfair::utils::logMessage(logfilename,msg);
            }
            else
            {
                error = true;
                info = "TBetfairmarket::addPlacedBet() ERROR - Bet ID " + betid + " is already present in failedbet list!";
                betfair::utils::logMessage(logfilename,info);
            }
        }
    }
    else
    {
        error = true;
        info = "TBetfairmarket::addPlacedBet() ERROR - null TBet pointer passed to function!";
        betfair::utils::logMessage(logfilename,info);
    }
    return !error;
}

} // end of namespace betfair
