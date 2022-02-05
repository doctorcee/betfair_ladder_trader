#ifndef JSON_QT_UTILS_H
#define JSON_QT_UTILS_H

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include "betfair/marketinfo.h"
#include "betfair/betfairmarket.h"


namespace betfair {
namespace json_utils {
namespace qt {

// functor for finding markets by string ID
struct marketInfoIDComparison {
    explicit marketInfoIDComparison(const QString& id) : my_id(id) { }
    inline bool operator()(const betfair::TMarketInfo& market) const { return (my_id == market.getMarketID());}
private:
    QString my_id;
};

TRunnerMetaInfo extractRunnerMetaInfo(const QJsonObject& runner);
std::map<double,double> extractPriceInfo(const QJsonArray& data);

bool extractDetailedMarketInfo(const QByteArray& data,
                               std::vector<betfair::TMarketInfo>& v_markets,
                               QString& error_string);

bool parseMarketSnapshot(const QByteArray& data,
                         betfair::TBetfairMarket& market,
                         QString& error_string);


}   // end of namespace qt
}   // end of namespace json_utils
}   // end of namespace betfair




#endif // JSON_QT_UTILS_H
