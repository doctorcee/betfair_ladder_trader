#include "ladderviewmodel.h"
#include "betfair/bet.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <sstream>
#include <initializer_list>


static const std::initializer_list<int> disproles = {Qt::DisplayRole,Qt::ForegroundRole,Qt::BackgroundRole,Qt::FontRole,Qt::TextAlignmentRole,Qt::UserRole};
static const QVector<int> vroles(disproles);

//=================================================================
LadderViewModel::LadderViewModel(QObject *parent,
                                 betfair::TBetfairMarket& mkt,
                                 const std::uint16_t& disptheme)
    : QAbstractTableModel(parent),
      m_market(mkt),
      m_selection_id(0),
      m_odds_vector(betfair::utils::generateOddsTickVector()),
      m_row_count(static_cast<int>(m_odds_vector.size())),
      m_col_count(ladderview::COLUMN_COUNT),
      m_history_seconds_hi(60),
      m_history_seconds_lo(30),
      m_display_theme(disptheme),
      m_last_lpm_row(-1),
      m_lay_money_colour{betfair::utils::layTwo,betfair::utils::layLadderFlash,Qt::black},
      m_suspended_colour{betfair::utils::susp,Qt::black},
      m_odds_colour{betfair::utils::darkgrey1,Qt::white},
      m_odds_traffic_light_colours{betfair::utils::red1,betfair::utils::gold1,betfair::utils::green1},
      m_back_money_colour{betfair::utils::backTwo,betfair::utils::backLadderFlash,Qt::black},
      m_hedge_colours{betfair::utils::redTwo,betfair::utils::midgrey1,betfair::utils::green1},
      m_betting_enabled(false)
{

    if (1 == m_display_theme)
    {
        m_back_money_colour[0] = betfair::utils::green1;
        m_back_money_colour[1] = betfair::utils::green1;
        m_back_money_colour[0].setAlpha(0xaa);
        m_back_money_colour[2] = Qt::white;
        m_lay_money_colour[0] = betfair::utils::red1;
        m_lay_money_colour[1] = betfair::utils::red1;
        m_lay_money_colour[0].setAlpha(0xaa);
        m_lay_money_colour[2] = Qt::white;
    }
}

//============================================================
void LadderViewModel::setSelectedRunner(const std::int64_t& runner_id)
{
    // look up this ID in the selection map
    if (m_market.valid())
    {
        m_selection_id = runner_id;
    }
    else
    {
        m_selection_id = 0;
    }
}


//=================================================================
void LadderViewModel::refresh()
{
    if (m_market.valid())
    {
        beginResetModel();
        endResetModel();
    }
}


//=================================================================
int LadderViewModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_row_count;
}

//=================================================================
int LadderViewModel::columnCount(const QModelIndex & /*parent*/) const
{
    return m_col_count;
}

//=================================================================
QString LadderViewModel::getSelectedMarketID() const
{
    return m_market.valid() ? m_market.getMarketID() : "";
}

//=================================================================
int LadderViewModel::getLastLPMRow() const
{
    int index = 0;
    if (m_market.valid() && m_selection_id != 0)
    {
        auto runner = m_market.getSelectionByID(m_selection_id);
        if (runner)
        {
            double lpm = runner->getLPM();
            if (lpm > 0.0)
            {
                for (auto it = m_odds_vector.begin(); it != m_odds_vector.end(); ++it)
                {
                    double odds = it->toDouble();
                    if (odds <= (lpm + 1e-6) && odds >= (lpm - 1e-6))
                    {
                        break;
                    }
                    ++index;
                }
            }
        }
    }
    return index;
}

//=================================================================
void LadderViewModel::updateVisibleData(const QModelIndex& topLeft,
                                        const QModelIndex& bottomRight)
{
    if (m_market.valid())
    {                
        emit dataChanged(topLeft, bottomRight,vroles);
    }
}

//=================================================================
void LadderViewModel::updateAllData()
{
    if (m_market.valid())
    {
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex(m_row_count,m_col_count);
        emit dataChanged(topLeft, bottomRight,vroles);
    }
}



//=================================================================
QVariant LadderViewModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    QString display_val = "";
    std::shared_ptr<betfair::TRunner> runner;
    if (row >= 0 && col >= 0 && row < m_row_count && col < m_col_count)
    {
        bool b_suspended = (m_market.getMarketStatus() == "SUSPENDED");
        const QString odds = m_odds_vector[static_cast<std::size_t>(row)];
        const double search_price = odds.toDouble();
        if (m_market.valid() && m_selection_id != 0)
        {
            runner = m_market.getSelectionByID(m_selection_id);
        }
        switch(role)
        {
            case Qt::DisplayRole:
                if (ladderview::ODDS == col)
                {
                    // Fixed odds
                    display_val = odds;
                }
                else
                {
                    if (runner)
                    {
                        if (ladderview::HEDGE == col)
                        {
                            double hedge = m_market.calculateRunnerHedgeProfitAtFixedOdds(m_selection_id,
                                                                                          m_market.getMarketID(),
                                                                                          search_price);
                            display_val = QString::number(hedge,'f',2);
                        }
                        else if (ladderview::LAY_BET == col)
                        {
                            // Display UNMATCHED totals for LAY bets at odds
                            double um = m_market.getRunnerUnmatchedVolAtOdds(m_selection_id, search_price, true);
                            if (um > 0.0)
                            {
                                display_val = QString::number(um,'f',1);
                            }
                        }
                        else if (ladderview::LAY_AV == col)
                        {
                            // TO Back prices
                            double available = runner->getAvailableToBackAtOdds(search_price);
                            if (available > 0.0)
                            {
                                display_val = QString::number(available,'f',0);
                            }
                        }
                        else if (ladderview::BACK_AV == col)
                        {
                            double available = runner->getAvailableToLayAtOdds(search_price);
                            if (available > 0.0)
                            {
                                display_val = QString::number(available,'f',0);
                            }
                        }
                        else if (ladderview::BACK_BET == col)
                        {
                            // Display UNMATCHED totals for BACK bets at odds
                            double um = m_market.getRunnerUnmatchedVolAtOdds(m_selection_id, search_price, false);
                            if (um > 0.0)
                            {
                                display_val = QString::number(um,'f',1);
                            }
                        }
                        else if (ladderview::TRADED == col)
                        {
                            double matched = runner->getTotalTraderVolumeAtOdds(search_price);
                            if (matched > 0.0)
                            {
                                display_val = QString::number(matched,'f',0);
                                matched /= 1000.0;
                                display_val = QString::number(matched,'f',1) + "K";
                            }
                        }
                    }
                }
                return display_val;
            case Qt::FontRole:
                if (ladderview::ODDS == col)
                {
                    QFont boldFont;
                    boldFont.setBold(true);
                    return boldFont;
                }               
                break;
            case Qt::ForegroundRole:

                if (ladderview::HEDGE == col)
                {                  
                    return QVariant::fromValue(QColor(Qt::white));
                }
                else if (ladderview::LAY_AV == col)
                {
                    return QVariant::fromValue(m_lay_money_colour[2]);
                }               
                else if (ladderview::ODDS == col)
                {
                    return QVariant::fromValue(m_odds_colour[1]);
                }
                else if (ladderview::BACK_AV == col)
                {
                    return QVariant::fromValue(m_back_money_colour[2]);
                }
                else if (ladderview::TRADED == col)
                {                    
                    // traded volume last X seconds column
                    double opacity = 0.0;
                    if (runner)
                    {                        
                        double max = runner->getHighestOddsVolumeTraded();
                        double matched_last_x = runner->getTotalTraderVolumeAtOdds(search_price, m_history_seconds_lo);
                        if (max > 0.0)
                        {
                            opacity = matched_last_x/max;
                        }
                    }
                    return QVariant::fromValue(opacity);
                }

                break;
            case Qt::BackgroundRole:            
                if (ladderview::HEDGE == col)
                {
                    // Profit/loss green up potential at this price
                    double hedge = m_market.calculateRunnerHedgeProfitAtFixedOdds(m_selection_id,
                                                                                  m_market.getMarketID(),
                                                                                  search_price);
                    if (hedge < 0.0)
                    {
                        return QVariant::fromValue(m_hedge_colours[0]);
                    }
                    else if (hedge > 0.0)
                    {
                        return QVariant::fromValue(m_hedge_colours[2]);
                    }
                    else
                    {
                        return QVariant::fromValue(m_hedge_colours[1]);
                    }
                }
                else if (ladderview::LAY_BET == col)
                {
                    return (m_betting_enabled == false) ? QVariant::fromValue(betfair::utils::midgrey1) : QVariant::fromValue(QColor(Qt::transparent));
                }
                else if (ladderview::BACK_BET == col)
                {
                    return (m_betting_enabled == false) ? QVariant::fromValue(betfair::utils::midgrey1) : QVariant::fromValue(QColor(Qt::transparent));
                }
                else if (ladderview::LAY_AV == col)
                {
                    // Did the most recent candle match at this price?
                    QColor t = m_lay_money_colour[0];
                    if (runner)
                    {
                        auto candles = runner->getAdvCandleData();
                        if (candles.empty() == false)
                        {                            
                            auto matchedvol = candles.rbegin()->getTradedVolume();
                            if (matchedvol.find(search_price) != matchedvol.end())
                            {
                                // we must have matched at this price in the last update
                                t = m_lay_money_colour[1];
                            }
                        }
                    }
                    return b_suspended ? QBrush(m_suspended_colour[0]) : QBrush(t);
                }
                else if (ladderview::ODDS == col)
                {
                    QBrush t(m_odds_colour[0]);
                    if (runner)
                    {
                        double lpm = runner->getLPM();
                        double od = odds.toDouble();
                        if (od <= (lpm + 1e-6) && od >= (lpm - 1e-6))
                        {
                            int lpmdir = runner->getLPMDirection();
                            if (lpmdir > 0)
                            {
                                t = QBrush(m_odds_traffic_light_colours[2]);
                            }
                            else if (lpmdir < 0)
                            {
                                t = QBrush(m_odds_traffic_light_colours[0]);
                            }
                            else
                            {
                                t = QBrush(m_odds_traffic_light_colours[1]);
                            }
                            m_last_lpm_row = row;
                        }
                    }
                    return t;
                }
                else if (ladderview::BACK_AV == col)
                {
                    // Did the most recent candle match at this price?
                    QColor t = m_back_money_colour[0];
                    if (runner)
                    {
                        auto candles = runner->getAdvCandleData();
                        if (candles.empty() == false)
                        {                            
                            auto matchedvol = candles.rbegin()->getTradedVolume();
                            if (matchedvol.find(search_price) != matchedvol.end())
                            {
                                // we must have matched at this price in the last update
                                t = m_back_money_colour[1];
                            }
                        }
                    }
                    return b_suspended ? QBrush(m_suspended_colour[0]) : QBrush(t);
                }
                else if (ladderview::TRADED == col)
                {
                    // traded volume column
                    if (runner)
                    {
                        double max = runner->getHighestOddsVolumeTraded();
                        double matched = runner->getTotalTraderVolumeAtOdds(search_price);
                        if (matched > 0.0)
                        {
                            if (max > 0.0)
                            {
                                const double opacity = matched/max;
                                return QVariant::fromValue(opacity);
                            }
                        }
                    }
                }
                break;
            case Qt::TextAlignmentRole:                
                return Qt::AlignHCenter + Qt::AlignVCenter;
            case Qt::UserRole:
                if (ladderview::TRADED == col)
                {
                    // traded volume last X seconds column
                    double opacity = 0.0;
                    if (runner)
                    {                        
                        double max = runner->getHighestOddsVolumeTraded();
                        double matched_last_60 = runner->getTotalTraderVolumeAtOdds(search_price, m_history_seconds_hi);
                        if (max > 0.0)
                        {
                            opacity = matched_last_60/max;
                        }
                    }
                    return QVariant::fromValue(opacity);
                }
                break;
            default:
                break;
        }
    }
    return QVariant();
}

