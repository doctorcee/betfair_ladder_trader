#include "json_qt_utils.h"
#include "betfair/runnermetainfo.h"


namespace betfair {
namespace json_utils {
namespace qt {


//====================================================================
TRunnerMetaInfo extractRunnerMetaInfo(const QJsonObject& runner)
{
    TRunnerMetaInfo info;
    info.id = runner.value("selectionId").toInt(0);
    info.name = runner.value("runnerName").toString("");
    QJsonObject metadata = runner.value("metadata").toObject();
    info.age = metadata.value("AGE").toString("");
    info.weight_units = metadata.value("WEIGHT_UNITS").toString("");
    info.weight_value = metadata.value("WEIGHT_VALUE").toString("");
    info.f_sp_num = metadata.value("FORECASTPRICE_NUMERATOR").toString("");
    info.f_sp_denom = metadata.value("FORECASTPRICE_DENOMINATOR").toString("");
    info.off_rating = metadata.value("OFFICIAL_RATING").toString("");
    info.sex_type = metadata.value("SEX_TYPE").toString("");
    info.days_since_last_run = metadata.value("DAYS_SINCE_LAST_RUN").toString("");
    info.wearing_type = metadata.value("WEARING").toString("");
    info.adj_rating = metadata.value("ADJUSTED_RATING").toString("");
    info.form_string = metadata.value("FORM").toString("");
    info.stall_draw = metadata.value("STALL_DRAW").toString("");
    info.jockey = metadata.value("JOCKEY_NAME").toString("");
    info.trainer = metadata.value("TRAINER_NAME").toString("");
    info.jockey_claim = metadata.value("JOCKEY_CLAIM").toString("");
    info.colours_filename = metadata.value("COLOURS_FILENAME").toString("");
    info.sort_priority = runner.value("sortPriority").toInt(0);
    return info;
}

//====================================================================
std::map<double,double> extractPriceInfo(const QJsonArray& data)
{
    // Function used to extract price data (odds/vol) from any of the
    // JSON arrays that contain pairs of price/size data (back/lay and
    // traded volume history).
    std::map<double,double> m_info;
    foreach (const QJsonValue& item, data)
    {
        QJsonObject curr_price = item.toObject();
        double odds = curr_price.value("price").toDouble(0);
        double vol = curr_price.value("size").toDouble(0);
        m_info.insert(std::make_pair(odds,vol));
    }
    return m_info;
}

//====================================================================
bool extractDetailedMarketInfo(const QByteArray& data,
                               std::vector<betfair::TMarketInfo>& v_markets,
                               QString& error_string)
{
    bool success = false;
    v_markets.clear();
    error_string = "";
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI
        error_string = jerror.errorString();
    }
    else
    {
        success = true;
        QJsonArray market_array = jdoc.array();
        //QJsonObject jsonObject = jdoc.object();
        //QJsonArray market_array = jsonObject.value("result").toArray();
        foreach (const QJsonValue& market, market_array)
        {
            QJsonObject curr_market = market.toObject();
            QJsonObject event = curr_market.value("event").toObject();
            QJsonObject event_type = curr_market.value("eventType").toObject();
            QJsonObject desc = curr_market.value("description").toObject();
            const QString mkid = curr_market.value("marketId").toString("");
            if (false == mkid.isEmpty())
            {
                TMarketInfo market_info;
                const QString mkname = curr_market.value("marketName").toString("");
                const QString evname = event.value("name").toString("");
                const QString venue = event.value("venue").toString("");
                const QString event_typeid = event_type.value("id").toString("");
                const QString event_typename = event_type.value("name").toString("");
                const QString start_time = curr_market.value("marketStartTime").toString("");               
                const QDateTime race_start = QDateTime::fromString(start_time,"yyyy-MM-ddTHH:mm:ss.zzzZ");
                const QString short_stime = race_start.toString("HH:mm");
                const bool ip = desc.value("turnInPlayEnabled").toBool();
                market_info.setGoesInplayFlag(ip);
                market_info.setMarketID(mkid);
                market_info.setMarketName(mkname);
                market_info.setStartTime(race_start);

                market_info.setVenue(venue);
                market_info.setEventTypeID(event_typeid);
                market_info.setEventTypeName(event_typename);
                QJsonArray runner_array = curr_market.value("runners").toArray();
                foreach (const QJsonValue& runner, runner_array)
                {
                    QJsonObject curr_runner = runner.toObject();
                    betfair::TRunnerMetaInfo r_info = extractRunnerMetaInfo(curr_runner);
                    if (false == market_info.insertSelection(r_info))
                    {
                        success = false;
                        error_string = ("betfair::json_utils::qt::extractDetailedMarketInfo() error - unable to insert TRunnerMetaInfo into TMarketInfo object.");                        
                    }
                }
                market_info.setEventName(evname);
                const QString race_type = desc.value("raceType").toString();
                const QString mtype = desc.value("marketType").toString();
                market_info.setMarketDetails(QString(mtype + " | " + race_type));
                if (v_markets.end() == std::find_if(v_markets.begin(),v_markets.end(),marketInfoIDComparison(mkid)))
                {
                    v_markets.push_back(market_info);
                }


            }
            else
            {                
                error_string = ("betfair::json_utils::qt::extractDetailedMarketInfo() error - no valid market ID found.");
                success = false;
                break;
            }
        }
        if (false == success)
        {
            v_markets.clear();
        }
    }
    return success;

}

//====================================================================
bool parseMarketSnapshot(const QByteArray& data,
                         betfair::TBetfairMarket& market,
                         QString& error_string)
{
    bool success = false;
    error_string = "";
    if (market.valid())
    {
        QJsonParseError jerror;
        QJsonDocument jdoc = QJsonDocument::fromJson(data,&jerror);
        if (jerror.error != QJsonParseError::NoError)
        {
            // Get error string and store for GUI
            error_string = jerror.errorString();
        }
        else
        {
            // This is the response from a listMarketBook attempt.
            QJsonArray jsonArray = jdoc.array();
            foreach (const QJsonValue& value, jsonArray)
            {
                // Extract market ID and look for it in our market list
                QJsonObject curr_market = value.toObject();
                const QString mk_id = curr_market.value("marketId").toString();
                if (mk_id == market.getMarketID())
                {
                    // Found market ID for selected market
                    success = true;

                    QDateTime tnow = QDateTime::currentDateTimeUtc();
                    const double tsnow_as_double = static_cast<double>(tnow.toUTC().toMSecsSinceEpoch());
                    const std::int64_t version = static_cast<std::int64_t>(curr_market.value("version").toVariant().toLongLong());
                    const bool cm = curr_market.value("crossMatching").toBool();
                    const int bet_delay = curr_market.value("betDelay").toInt(-1);
                    const double total_market_matched = curr_market.value("totalMatched").toDouble();
                    const int num_active_runners = curr_market.value("numberOfActiveRunners").toInt(-1);
                    const QString market_status = curr_market.value("status").toString("error");
                    const bool market_suspended = market_status == "SUSPENDED";
                    const bool market_closed = market_status == "CLOSED";

                    bool b_inplay = curr_market.value("inplay").toBool();

                    const QString last_market_matchtime = curr_market.value("lastMatchTime").toString("error");
                    const QDateTime lastmts = QDateTime::fromString(last_market_matchtime,"yyyy-MM-ddTHH:mm:ss.zzzZ");
                    if (!lastmts.isValid())
                    {
                        error_string.append("QDateTime created from (\"lastMatchTime\":\"" + last_market_matchtime + "\") is invalid!\n");
                    }
                    const double last_market_match_ts = (lastmts.isValid()) ? static_cast<double>(lastmts.toUTC().toMSecsSinceEpoch()) : tsnow_as_double;

                    market.setMarketStatus(market_status);
                    market.setBetDelay(bet_delay);
                    market.setInPlay(b_inplay);
                    if (version < market.getMarketVersion())
                    {
                        error_string.append("WARNING: Most recent market version (" + QString::number(version) + " is less than previous value (");
                        error_string.append(QString::number(market.getMarketVersion()) + ")\n");
                    }
                    market.updateMarketVersion(version);
                    market.updateCrossMatchedState(cm);

                    bool nonrunner_removal = false;
                    bool time_error = false;
                    if (last_market_match_ts < market.getLastMatchedTimestamp())
                    {
                        // THIS response has a last match timestamp EARLIER than the previous

                        if (false == (market_closed || market_suspended))
                        {
                            // PREVENT SNAPSHOT CREATION HERE!
                            error_string.append("New snapshot last match timestamp is earlier than the current one for this market! Ignoring response.\n");
                            time_error = true;
                        }
                    }
                    market.updateLastMatchedTimestamp(last_market_match_ts);
                    int current_active_runner_count = market.getReportedActiveRunnerCount();
                    if (num_active_runners > 0)
                    {
                        // update and then check for non-runner
                        market.updateReportedActiveRunnerCount(num_active_runners);
                    }
                    if ((current_active_runner_count > 0) && (num_active_runners > 0) && (num_active_runners < current_active_runner_count))
                    {
                        // This must be a non-runner getting removed
                        // We will probably get adjusted prices
                        error_string.append("Ignoring this response - non-runner removal detected\n");
                        nonrunner_removal = true;
                    }
                    if (time_error)
                    {
                        // PREVENT SNAPSHOT CREATION HERE!
                        return false;
                    }
                    if (nonrunner_removal)
                    {                       
                        return true;
                    }
                    market.updateMarketTotalMatched(tsnow_as_double, total_market_matched);
                    QJsonArray runners_array = curr_market.value("runners").toArray();
                    foreach (const QJsonValue& runner, runners_array)
                    {
                        QJsonObject curr_runner = runner.toObject();
                        const QString runner_status = curr_runner.value("status").toString("error");
                        const std::int64_t runner_id = static_cast<std::int64_t>(curr_runner.value("selectionId").toInt(-1));

                        const double lpm = curr_runner.value("lastPriceTraded").toDouble(0);
                        const double adj = curr_runner.value("adjustmentFactor").toDouble(0);
                        const double runner_total_matched = curr_runner.value("totalMatched").toDouble(0);

                        QJsonObject exchange_prices = curr_runner.value("ex").toObject();

                        QJsonArray back_price_array = exchange_prices.value("availableToBack").toArray();
                        QJsonArray lay_price_array = exchange_prices.value("availableToLay").toArray();
                        QJsonArray traded_volume_array = exchange_prices.value("tradedVolume").toArray();

                        std::shared_ptr<TMarketSnapshot> new_snap = std::make_shared<TMarketSnapshot>(mk_id,
                                                                                                      runner_id,
                                                                                                      tsnow_as_double,
                                                                                                      bet_delay,
                                                                                                      lpm,
                                                                                                      adj,
                                                                                                      runner_total_matched,
                                                                                                      runner_status,
                                                                                                      market_status,
                                                                                                      last_market_match_ts);
                        new_snap->setBackOrders(extractPriceInfo(back_price_array));
                        new_snap->setLayOrders(extractPriceInfo(lay_price_array));
                        new_snap->setTradedHistory(extractPriceInfo(traded_volume_array));
                        QString runner_add_error = "";
                        if (false == market.updateRunnerSnapshot(runner_id,new_snap,runner_add_error))
                        {
                            success = false;
                            error_string.append(runner_add_error + "\n");
                        }                       
                    }
                    break;
                }
            }
        }
    }
    else
    {
        error_string = "TBetfairMarket is invalid!";
    }
    return success;
}

} // end of namespace qt
} // end of namespace json_utils
} // end of namespace betfair
