#include "runner.h"

namespace betfair {

//=================================================
TRunner::TRunner(const TRunnerMetaInfo& info)
    : m_info(info),
      m_current_snapshot(nullptr),
      m_y_min(0.0),
      m_y_max(0.0),
      m_profit(0.0),    
      m_higest_traded_price_vol(0.0),
      m_lpm_move_state(0)
{
    m_adv_candle_data.reserve(1000);
}

//=================================================
void TRunner::updateProfitIfWins(const std::int64_t& id,
                                 const double& profit)
{
    if (id == m_info.id)
    {
        m_profit = profit;      
    }
}

//=================================================
QString TRunner::getStatus() const
{
    if (m_current_snapshot)
    {
        return m_current_snapshot->getStatus();
    }
    else
    {
        return "UNKNOWN";
    }
}

//=================================================
double TRunner::getHighestOddsVolumeTraded() const
{
    return m_higest_traded_price_vol;
}

//=================================================
double TRunner::getReductionFactor() const
{
    if (m_current_snapshot)
    {
        return m_current_snapshot->getAdjustmentFactor();
    }
    else
    {
        return -1.0;
    }
}

//=================================================
double TRunner::getTotalMatched() const
{
    if (m_current_snapshot)
    {
        return m_current_snapshot->getTotalVolumeMatched();
    }
    else
    {
        return 0.0;
    }
}

//=================================================
void TRunner::getLatestIntervalData(double& min,
                                    double& max,
                                    double& lpm)
{
    min = 0.0;
    max = 0.0;
    lpm = 0.0;
    if (false == m_adv_candle_data.empty())
    {
        auto last_candle = m_adv_candle_data.rbegin();
        min = last_candle->getLow();
        max = last_candle->getHigh();
        lpm = last_candle->getClose();
    }
}

//=================================================
double TRunner::getLPM() const
{
    if (m_current_snapshot)
    {
        return m_current_snapshot->getLPM();
    }
    else
    {
        return -1.0;
    }
}

//=================================================
bool TRunner::isActive() const
{
    if (m_current_snapshot)
    {
        return ("ACTIVE" == m_current_snapshot->getStatus());
    }
    else
    {
        return false;
    }
}

//=================================================
bool TRunner::isWinner() const
{
    if (m_current_snapshot)
    {
        return ("WINNER" == m_current_snapshot->getStatus());
    }
    else
    {
        return false;
    }
}


//=================================================
double TRunner::getDataStartTime() const
{
    if (m_adv_candle_data.empty())
    {
        return 0.0;
    }
    else
    {
        return m_adv_candle_data.begin()->start_timestamp;
    }
}

//=================================================
double TRunner::getDataEndTime() const
{
    if (m_adv_candle_data.empty())
    {
        return 0.0;
    }
    else
    {
        return m_adv_candle_data.rbegin()->start_timestamp;
    }
}

//=================================================
std::map<double,double> TRunner::getAvailableBackPrices() const
{
    if (m_current_snapshot)
    {
        return m_current_snapshot->getBackOrders();
    }
    else
    {
        return std::map<double,double>();
    }
}
//=================================================
std::map<double,double> TRunner::getAvailableLayPrices() const
{
    if (m_current_snapshot)
    {
        return m_current_snapshot->getLayOrders();
    }
    else
    {
        return std::map<double,double>();
    }
}

//=================================================
std::pair<double,double> TRunner::getOrderedBackPrice(int index) const
{
    // NOTE INDEX OPERATES FROM THE LAST ITEM IN THE MAP TO ALLOW
    // GETTING NTH BEST LAY BACK AVAILABLE
    std::pair<double,double> retval = std::make_pair(-1.0,-1.0);
    if (m_current_snapshot)
    {
        // This is crude and lumpy but maps are not random access
        auto back_prices = m_current_snapshot->getBackOrders();
        int counter = 0;
        // NOTE WE OPERATE WITH REVERSE ITERATOR.
        for (auto it = back_prices.rbegin(); it != back_prices.rend(); ++it)
        {
            if (counter == index)
            {
                retval.first = it->first;
                retval.second = it->second;
                break;
            }
            ++counter;
        }
    }
    return retval;
}

//=================================================
std::pair<double,double> TRunner::getOrderedLayPrice(int index) const
{
    std::pair<double,double> retval = std::make_pair(-1.0,-1.0);
    if (m_current_snapshot)
    {
        // This is crude and lumpy but maps are not random access        
        auto prices = m_current_snapshot->getLayOrders();
        int counter = 0;
        for (auto it = prices.begin(); it != prices.end(); ++it)
        {
            if (counter == index)
            {
                retval.first = it->first;
                retval.second = it->second;
                break;
            }
            ++counter;
        }
    }
    return retval;
}

//=================================================
double TRunner::getAvailableToBackAtOdds(const double odds)
{
    double retval = 0.0;
    if (m_current_snapshot)
    {
        auto backprices = m_current_snapshot->getBackOrders();
        auto it = backprices.find(odds);
        if (it != backprices.end())
        {
            retval = it->second;
        }
    }
    return retval;
}

//=================================================
double TRunner::getAvailableToLayAtOdds(const double odds)
{
    double retval = 0.0;
    if (m_current_snapshot)
    {
        auto prices = m_current_snapshot->getLayOrders();
        auto it = prices.find(odds);
        if (it != prices.end())
        {
            retval = it->second;
        }
    }
    return retval;
}

//=================================================
double TRunner::getTotalTraderVolumeAtOdds(const double odds,
                                           const std::uint16_t last_x_seconds)
{
    double retval = 0.0;
    QDateTime tnow = QDateTime::currentDateTimeUtc();
    int offset = static_cast<int>(last_x_seconds);
    tnow = tnow.addSecs(-offset);
    const double threshtime = static_cast<double>(tnow.toUTC().toMSecsSinceEpoch());
    // TODO: scan the last X seconds worth of candles
    for (auto it = m_adv_candle_data.rbegin(); it != m_adv_candle_data.rend(); ++it)
    {
        if (it->end_timestamp < threshtime)
        {
            break;
        }
        else
        {
            auto candle_trade_vol = it->getTradedVolume();
            auto pit = candle_trade_vol.find(odds);
            if (pit != candle_trade_vol.end())
            {
                retval += pit->second;
            }
        }
    }
    return retval;

}

//=================================================
double TRunner::getTotalTraderVolumeAtOdds(const double odds)
{
    double retval = 0.0;
    if (m_current_snapshot)
    {
        auto trade = m_current_snapshot->getTradeHistory();
        auto it = trade.find(odds);
        if (it != trade.end())
        {
            retval = it->second;
        }
    }
    return retval;
}

//=================================================
double TRunner::getCurrentAvailableLayVolume(int N) const
{
    // Get total volume of N BEST available prices
    double vol = 0.0;
    int counter = 0;
    if (m_current_snapshot && (N > 0))
    {
        auto backprices = m_current_snapshot->getLayOrders();
        for (auto it = backprices.begin(); it != backprices.end(); ++it)
        {
            vol += it->second;
            ++counter;
            if (counter >= N)
            {
                break;
            }
        }
    }
    return vol;
}

//=================================================
double TRunner::getCurrentAvailableBackVolume(int N) const
{
    // Get total volume of 10 BEST available prices
    double vol = 0.0;
    int counter = 0;
    if (m_current_snapshot && (N > 0))
    {
        auto prices = m_current_snapshot->getBackOrders();
        for (auto it = prices.rbegin(); it != prices.rend(); ++it)
        {
            vol += it->second;
            ++counter;
            if (counter >= N)
            {
                break;
            }
        }
    }
    return vol;
}

//=================================================
bool TRunner::update(std::shared_ptr<TMarketSnapshot> new_snapshot,
                     QString& error_msg)
{
    // We now basically only create a MEANINGFUL candle whenever
    // we have two market status == OPEN snapshots. Too complicated dealing
    // with market adjustments and rogue match values when this is not the case.
    bool success = false;
    m_higest_traded_price_vol = 0.0;
    if (new_snapshot)
    {
        if (new_snapshot->getSelectionID() == m_info.id)
        {
            success = true;
            if (m_current_snapshot)
            {
                double new_lpm = new_snapshot->getLPM();
                double old_lpm = m_current_snapshot->getLPM();
                if (old_lpm > new_lpm)
                {
                    // price decrease
                    m_lpm_move_state = -1;
                }
                else if (old_lpm < new_lpm)
                {
                    // price increase
                    m_lpm_move_state = 1;
                }
                else
                {
                    m_lpm_move_state = 0;
                }
                // already have a snapshot
                // Calculate new candle from current and new snapshot
                // PERFORM A MARKET TIMESTAMP CHECK
                if (m_current_snapshot->getLastMarketMatchedBetTimestamp() > new_snapshot->getLastMarketMatchedBetTimestamp())
                {
                    // This cannot be right. The new TS should NEVER be earlier then the previous!
                    success = false;
                    error_msg.append("New snapshot last matched timestamp is earlier than the current one!\n");
                }
                else
                {
                    QString candle_errors = "";                   
                    betfair::utils::TBetfairAdvancedCandle adv_candle = betfair::utils::generateAdvancedCandle(m_info.id,m_current_snapshot,new_snapshot);

                    if (adv_candle.price_anomalies.size() > 0)
                    {
                        for (const std::string& err_str : adv_candle.price_anomalies)
                        {
                            error_msg.append(QString::fromStdString(err_str) + "\n");
                        }
                    }
                    m_adv_candle_data.push_back(adv_candle);
                    const double hi = adv_candle.getHigh();
                    const double lo = adv_candle.getLow();
                    const double vol = adv_candle.getCandleVol();
                    const double cumvwap = adv_candle.getCumulativeVWAP();
                    if (vol > 0.0)
                    {
                        if (hi > m_y_max)
                        {
                            m_y_max = hi;
                        }
                        if (lo < m_y_min)
                        {
                            m_y_min = lo;
                        }
                        if (cumvwap > m_y_max)
                        {
                            m_y_max = cumvwap;
                        }
                        if (cumvwap < m_y_min)
                        {
                            m_y_min = cumvwap;
                        }
                    }
                }
            }
            else
            {
                // first time              
                m_adv_candle_data.clear();
                m_adv_candle_data.reserve(1000);
            }
            m_current_snapshot = new_snapshot;            
            if (m_current_snapshot)
            {
                auto traded = m_current_snapshot->getTradeHistory();
                for (auto it = traded.begin(); it != traded.end(); ++it)
                {
                    if (it->second > m_higest_traded_price_vol)
                    {
                        m_higest_traded_price_vol = it->second;
                    }
                }
            }
        }
    }
    return success;
}

} // end of namespace betfair
