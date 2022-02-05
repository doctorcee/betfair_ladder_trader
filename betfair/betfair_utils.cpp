#include "betfair_utils.h"
#include "marketsnapshot.h"
#include "betfairadvancedcandle.h"
#include <sstream>
#include <cmath>
#include <QFile>
#include <QDateTime>
#include <QTextStream>

namespace betfair {
namespace utils {

//==============================================================
std::map<QString,std::uint32_t> generateOffsetMap(const std::map<std::uint32_t,QString>& mp)
{
    // Generate ordered betfair odds ladder map with ODDS STRING as key
    // and TICK INDEX ID as value
    std::map<QString,std::uint32_t> odds_map;
    for (auto it = mp.begin(); it != mp.end(); ++it)
    {
        odds_map[it->second] = it->first;
    }
    return odds_map;
}

//==============================================================
std::map<double,std::uint32_t> generatePriceDoubleTickMap()
{
    std::map<std::uint32_t,QString> tick_odds = generateOddsMap();
    std::map<QString,std::uint32_t> mp = generateOffsetMap(tick_odds);

    std::map<double,std::uint32_t> odds_map;
    for (auto it = mp.begin(); it != mp.end(); ++it)
    {
        odds_map.insert(std::make_pair(it->first.toDouble(),it->second));
    }
    return odds_map;
}

//==============================================================
std::vector<QString> generateOddsTickVector()
{
    std::vector<QString> v_odds;
    v_odds.reserve(600);
    double odds = 1.01;
    for (int i = 0; i < 99; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',2));
        odds += 0.01;
    }
    odds = 2.00;
    for (int i = 0; i < 50; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',2));
        odds += 0.02;
    }
    odds = 3.00;
    for (int i = 0; i < 20; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',2));
        odds += 0.05;
    }
    odds = 4.00;
    for (int i = 0; i < 20; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',1));
        odds += 0.1;
    }
    odds = 6.00;
    for (int i = 0; i < 20; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',1));
        odds += 0.2;
    }
    odds = 10.00;
    for (int i = 0; i < 20; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',1));
        odds += 0.5;
    }
    odds = 20.00;
    for (int i = 0; i < 10; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',0));
        odds += 1;
    }
    odds = 30.00;
    for (int i = 0; i < 10; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',0));
        odds += 2;
    }
    odds = 50.00;
    for (int i = 0; i < 10; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',0));
        odds += 5;
    }
    odds = 100.00;
    for (int i = 0; i < 91; ++i)
    {
        v_odds.push_back(QString::number(odds,'f',0));
        odds += 10;
    }
    std::reverse(v_odds.begin(),v_odds.end());
    return v_odds;
}

//==============================================================
std::map<std::uint32_t,QString> generateOddsMap()
{
    // Generate ordered betfair odds ladde map with TICK INDEX ID as key
    // and ODDS STRING as value
    std::map<std::uint32_t, QString> odds_map;
    std::uint32_t tick_index = 0;
    double odds = 1.01;
    for (int i = 0; i < 99; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 0.01;
        ++tick_index;
    }
    odds = 2.00;
    for (int i = 0; i < 50; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 0.02;
        ++tick_index;
    }
    odds = 3.00;
    for (int i = 0; i < 20; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 0.05;
        ++tick_index;
    }
    odds = 4.00;
    for (int i = 0; i < 20; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 0.1;
        ++tick_index;
    }
    odds = 6.00;
    for (int i = 0; i < 20; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 0.2;
        ++tick_index;
    }
    odds = 10.00;
    for (int i = 0; i < 20; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 0.5;
        ++tick_index;
    }
    odds = 20.00;
    for (int i = 0; i < 10; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 1;
        ++tick_index;
    }
    odds = 30.00;
    for (int i = 0; i < 10; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 2;
        ++tick_index;
    }
    odds = 50.00;
    for (int i = 0; i < 10; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 5;
        ++tick_index;
    }
    odds = 100.00;
    for (int i = 0; i < 91; ++i)
    {
        odds_map[tick_index] = QString::number(odds,'f',2);
        odds += 10;
        ++tick_index;
    }
    return odds_map;
}

//====================================================================
TBetfairAdvancedCandle createMergedCandle(const std::vector<TBetfairAdvancedCandle>& data)
{
    // This function takes a vector of advanced candles and returns a NEW candle that is the result of
    // merging them all together.
    const std::int64_t id = data.empty() ? 0 : data.begin()->getSelectionID();
    TBetfairAdvancedCandle candle(id);
    const std::size_t len = data.size();
    if (len > 0)
    {
        candle = data[0];
        for (std::size_t i = 1; i < len; ++i)
        {
            if (data[i].getSelectionID() != id)
            {
                throw std::runtime_error("Cannot merge empty candle vector - mismatching selection IDs encountered!");
            }
            candle.merge(data[i]);
        }
    }
    else
    {
        throw std::runtime_error("Cannot merge empty candle vector!");
    }
    return candle;
}

//====================================================================
std::vector<TBetfairAdvancedCandle> mergeCandlesByMinimumTimeInterval(const std::vector<TBetfairAdvancedCandle>& data,
                                                                      const double& millisecond_interval)
{
    std::vector<TBetfairAdvancedCandle> merged_data;
    if (millisecond_interval > 0.0)
    {
        if (!data.empty())
        {
            std::vector<TBetfairAdvancedCandle> to_merge; // container for candles to be merged to a single candle
            auto it = data.begin();
            double start_time = it->start_timestamp;
            ++it;
            while (it != data.end())
            {
                if ((start_time + millisecond_interval) < it->start_timestamp)
                {
                    // This candle goes past our interval end point so we merge ALL
                    // candles stored in the vector up until this point
                    auto new_candle = createMergedCandle(to_merge);
                    merged_data.push_back(new_candle);

                    // Clear the vector we just merged and current candle to it.
                    to_merge.clear();
                    start_time = it->start_timestamp;
                    to_merge.push_back(*it);
                }
                else
                {
                    // candle still within the interval so just add to the merge vector
                    to_merge.push_back(*it);
                }
                ++it;
            }
        }
    }
    return merged_data;
}

//====================================================================
std::vector<TBetfairAdvancedCandle> mergeCandles(const std::vector<TBetfairAdvancedCandle>& data,
                                                 const std::uint16_t& width)
{
    // Returns a vector of candles formed by merging each consecutive set of [width] candles
    // contained in the data vector
    std::vector<TBetfairAdvancedCandle> merged_data;
    if (width > 1)
    {
        const std::size_t len = data.size();
        std::size_t i = 0;
        while (i < len)
        {
            std::vector<TBetfairAdvancedCandle> to_merge;
            for (std::size_t j = 0; j < width; ++j)
            {
                if (i + j < len)
                {
                    to_merge.push_back(data[i+j]);
                }
            }
            if (!to_merge.empty())
            {
                auto new_candle = createMergedCandle(to_merge);
                merged_data.push_back(new_candle);
            }
            i += width;
        }
    }
    else if (1 == width)
    {
        for (auto it : data)
        {           
            merged_data.push_back(it);
        }
    }
    return merged_data;
}

//====================================================================
void calculateEMA(std::uint32_t width,
                  const std::vector<TBetfairAdvancedCandle>& candle_data,
                  std::vector<std::pair<double,double> >& ema_vector)
{
    // Populate reference vector of pairs of timestamp(std::string) and ema value (double)
    // Financial EMA use the close price traditionally, but we are going to use the weighted
    // matched average value here so it is more robust against small size matched price outliers
    ema_vector.clear();
    if (width > 0 && width < candle_data.size())
    {
        std::uint32_t counter = 0;
        const double alpha= 2.0/(1 + width);
        const double beta = 1.0 - alpha;
        double current_ema = 0.0;
        double prev_ema = 0.0;
        double sum = 0.0;
        for (auto it = candle_data.begin(); it != candle_data.end(); ++it)
        {
            ++counter;            
            auto ts = it->end_timestamp;            
            if (counter == width)
            {
                sum += it->getCandleVWAP();
                current_ema = sum/width;
                ema_vector.push_back(std::make_pair(ts,current_ema));
            }
            else if (counter < width)
            {
                sum += it->getCandleVWAP();
            }
            else if (counter > width)
            {
                // We have enough points now
                if (it->getCandleVWAP() > 0.0)
                {
                    current_ema = (alpha * it->getCandleVWAP()) + (beta * prev_ema);
                }
                else
                {
                    current_ema = prev_ema;
                }
                ema_vector.push_back(std::make_pair(ts,current_ema));
            }
            prev_ema = current_ema;
        }
    }
}


//====================================================================
void calculateMACD(const std::uint32_t& short_width,
                   const std::uint32_t& long_width,
                   const std::vector<TBetfairAdvancedCandle>& candle_data,
                   std::vector<std::pair<double,double> >& macd_vector)
{
    macd_vector.clear();
    if (short_width < long_width)
    {
        const std::size_t ct = candle_data.size();
        if (ct > long_width)
        {
            std::vector<std::pair<double,double> > ema_long;
            std::vector<std::pair<double,double> > ema_short;
            calculateEMA(long_width,candle_data,ema_long);
            calculateEMA(short_width,candle_data,ema_short);

            if (ema_long.size() + (long_width-1) != ct)
            {
                throw std::runtime_error("calculateMACD() error - data vector EMA lengths not consistent!");
            }
            if (ema_short.size() + (short_width-1) != ct)
            {
                throw std::runtime_error("calculateMACD() error - data vector EMA lengths not consistent!");
            }

            auto it_long = ema_long.begin();
            auto it_short = ema_short.begin() + (long_width - short_width);
            while (it_long != ema_long.end())
            {
                // Calculation of the actual MACD
                if (it_long->first != it_short->first)
                {
                    throw std::runtime_error("calculateMACD() error - iterators not aligned correctly!");
                }
                macd_vector.push_back(std::make_pair(it_long->first,it_short->second - it_long->second));
                ++it_long;
                ++it_short;
            }
            if (it_short != ema_short.end())
            {
                throw std::runtime_error("calculateMACD() error - final iterators do not match!");
            }
        }
    }
}


//====================================================================
TBetfairAdvancedCandle generateAdvancedCandle(const std::int64_t& selection_id,
                                               const std::shared_ptr<TMarketSnapshot>& a,
                                               const std::shared_ptr<TMarketSnapshot>& b)
{
    return TBetfairAdvancedCandle(selection_id,a,b);
}

//====================================================================
void calculateEMA(std::uint32_t width,
                  const std::vector<std::pair<double,double> >& t_y_data,
                  std::vector<std::pair<double,double> >& ema_vector)
{
    // Input data is of the form (timestamp,datavalue)
    ema_vector.clear();
    if (width > 0)
    {
        std::uint32_t counter = 0;
        const double alpha= 2.0/(1 + width);
        const double beta = 1.0 - alpha;
        double current_ema = 0.0;
        double prev_ema = 0.0;
        double sum = 0.0;
        for (auto it = t_y_data.begin(); it != t_y_data.end(); ++it)
        {
            ++counter;
            if (counter == width)
            {
                sum += it->second;
                current_ema = sum/width;
                ema_vector.push_back(std::make_pair(it->first,current_ema));
            }
            else if (counter < width)
            {
                sum += it->second;
            }
            else if (counter > width)
            {
                // We have enough points now.
                current_ema = (alpha * it->second) + (beta * prev_ema);
                ema_vector.push_back(std::make_pair(it->first,current_ema));
            }
            prev_ema = current_ema;

        }
    }
}



//==========================================================================
bool calculateTradedIntervalData(const std::map<double,double>& a_hist,
                                 const std::map<double,double>& b_hist,
                                 std::map<double,double>& interval_data,
                                 QString& error_string)
{
    bool b_error = false;
    interval_data.clear();
    for (auto it = b_hist.begin(); it != b_hist.end(); ++it)
    {
        // For each price search for the same price in a
        const double this_price = it->first;
        const double this_vol = it->second;
        auto ait = a_hist.find(this_price);
        if (ait != a_hist.end())
        {
            // This price is present in a - calculate the difference
            const double vol_diff = this_vol - ait->second;
            if (vol_diff > 0.0)
            {
                interval_data[this_price] = vol_diff;
            }
            else if (vol_diff < 0.0)
            {
                b_error = true;
                error_string.append("WARNING: Encountered negative matched volume - Price = " + QString::number(this_price,'f',2));
                error_string.append(", Previous volume = " + QString::number(ait->second,'f',2) + ", New volume = " + QString::number(this_vol,'f',2));
                error_string.append("\n");
            }
        }
        else
        {
            // not in a - therefore this is a first time price match
            if (this_vol > 0.0)
            {
                interval_data[this_price] = this_vol;
            }
        }
    }
    return !b_error;
}

//====================================================================
QString generateGenericLMCatFilter(const std::vector<QString>& v_countries,
                                   const std::vector<QString>& v_market_types,
                                   const QString& evtypeid,
                                   int market_count_limit,
                                   bool list_orders,
                                   bool today_only)
{
    // Generate payload JSON for customised markets.
    // Maximum allowable limit is 100 markets because we ask for metadata
    // ALWAYS!!!
    // Will ALWAYS ask for list by start time so however many markets asked for
    // they will be the next X chronologically ordered by start time!
    // Listing of orders too is not CURRENTLY supported

    QString qpayload = "";
    if (market_count_limit <= 100 && market_count_limit > 0 && evtypeid.length() > 0)
    {
        qpayload = "{\"filter\":{\"eventTypeIds\": [\"" + evtypeid + "\"]";
        if (v_market_types.empty() == false)
        {
            // Create marketTypeCodes list from input argument
            QString mtype_code = ",\"marketTypeCodes\":[";
            for (const QString& t : v_market_types)
            {
                mtype_code.append("\"" + t + "\",");
            }
            mtype_code.chop(1);
            mtype_code.append("]");
            qpayload.append(mtype_code);
        }
        if (v_countries.empty() == false)
        {
            // create marketCountries list from input argument
            QString country_code = ",\"marketCountries\":[";
            for (const QString& c : v_countries)
            {
                country_code.append("\"" + c + "\",");
            }
            country_code.chop(1);
            country_code.append("]");
            qpayload.append(country_code);
        }
        QDateTime now = QDateTime::currentDateTimeUtc();
        QDate sel_day = now.date();       
        QString start_time = sel_day.toString(QString("yyyy-MM-dd")) + QString("T00:00:01Z");
        qpayload.append(QString(",\"marketStartTime\":{\"from\":\""));
        qpayload.append(start_time);
        if (today_only)
        {
            // Add end time to marketStartTime instance to ensure we only
            // get markets that start TODAY
            QString end_time = sel_day.toString(QString("yyyy-MM-dd")) + QString("T23:59:59Z");
            qpayload.append(QString("\",\"to\":\""));
            qpayload.append(end_time);
        }
        qpayload.append(QString("\"}"));
        QString market_lim = QString::number(market_count_limit);
        qpayload.append(QString("},\"sort\":\"FIRST_TO_START\",\"maxResults\":\"" + market_lim + "\""));
        qpayload.append(QString(",\"marketProjection\":[\"MARKET_DESCRIPTION\",\"EVENT_TYPE\",\"RUNNER_METADATA\",\"MARKET_START_TIME\",\"EVENT\",\"COMPETITION\"]}"));
    }
    return qpayload;

}

//====================================================================
void logMessage(const QString& filename, const QString& msg)
{
    QFile file(filename);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;

    QString tnow = QDateTime::currentDateTimeUtc().toString("[yyyyMMMdd HH:mm:ss] ");
    QTextStream out(&file);
    out << tnow << msg << "\n";
    file.close();
}

} // end of namespace utils
} // end of namespace betfair

