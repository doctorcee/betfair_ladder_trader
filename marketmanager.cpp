//====================================================================
// Filename: marketmanager.cpp
// Version:
// Author:        Doctor C
//
// Created at:    12:00 2016/01/01
// Modified at:
// Description:   class to manage betfair API interaction and
//                market monitoring
//====================================================================


#include "marketmanager.h"
#include "betfair/bet.h"
#include "betfair/json_utils/qt/json_qt_utils.h"
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QAuthenticator>
#include <QFile>
#include <QNetworkRequest>
#include <QSslKey>
#include <QSsl>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <cmath>



class sortByStartTime {
public:
    bool operator()(betfair::TMarketInfo const &a, betfair::TMarketInfo const &b) {
        return (a.getStartTime() < b.getStartTime());
    }
};

static const QString current_orders_log = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_current_orders_logfile.txt";
static const QString api_log = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_API_log.txt";
static const QString network_errors_log = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_network_errors.txt";

static const QString loginurl("https://identitysso-cert.betfair.com/api/certlogin");

//====================================================================
MarketManager::MarketManager(betfair::TBetfairMarket& mkt)
    : m_market(mkt),
      m_netmanager(nullptr),
      m_username(""),
      m_password(""),
      m_cert_file(""),
      m_key_file(""),
      m_app_key(""),
      m_session_token(""),
      m_error_string(""),      
      m_log_file(""),
      m_error_file(""),      
      m_account_balance(-1.0),
      m_b_logged_in(false),
      m_log_responses_to_file(true)
{
    m_netmanager = new QNetworkAccessManager();
    QObject::connect(m_netmanager, SIGNAL(finished(QNetworkReply*)),
                        this,SLOT(replyFinished(QNetworkReply*)));
    QObject::connect(m_netmanager,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
                     this,SLOT(replySSLErrors(QNetworkReply*,QList<QSslError>)));
    QObject::connect(m_netmanager,SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                     this,SLOT(replyAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    QObject::connect(m_netmanager,SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&,QAuthenticator*)),
                     this,SLOT(replyProxyAuthenticationRequired(const QNetworkProxy&,QAuthenticator*)));

    m_log_file = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_placed_bets_logfile.txt";
    m_error_file = QDateTime::currentDateTime().toString("yyyyMMMdd") + "_error_log.txt";
}

//====================================================================
MarketManager::~MarketManager()
{
    // make sure we write any files that need writing to file
    writeLogMessageBufferToFile();
    writeAPIMessageBufferToFile();
    delete m_netmanager;
}

//====================================================================
void MarketManager::setLoginDetails(const QString& uname, const QString& passw,
                                    const QString& certfile, const QString& keyfile,
                                    const QString& appkey)
{
    m_username = uname;
    m_password = passw;
    m_cert_file = certfile;
    m_key_file = keyfile;
    m_app_key  = appkey;
}


//====================================================================
betfair::TMarketInfo MarketManager::getMarketInfoByID(const QString& id)
{
    auto it = std::find_if(m_market_info_list.begin(),m_market_info_list.end(),betfair::json_utils::qt::marketInfoIDComparison(id));
    if (it != m_market_info_list.end())
    {
        // Item found
        return *it;
    }
    else
    {
        return betfair::TMarketInfo();
    }
}


//====================================================================
bool MarketManager::isLoggedIn() const
{
    return m_b_logged_in;
}

//====================================================================
QString MarketManager::getErrorString() const
{
    return m_error_string;
}

//====================================================================
QString MarketManager::getSessionToken() const
{
    return m_session_token;
}

//====================================================================
QString MarketManager::getUname() const
{
    return m_username;
}


//====================================================================
bool MarketManager::attemptLogin()
{
    // Create a login request using current supplied login
    // credentials
    bool success = true;
    QByteArray payload("username=");
    payload.append(m_username);
    payload.append("&password=");
    payload.append(m_password);

    QString url("https://identitysso.betfair.com/api/certlogin");

    QFile file(m_cert_file);
    if(file.exists())
    {
        if(!file.open(QIODevice::ReadOnly))
        {
            QString err = "Login error - unable to open certificate file " + m_cert_file;
            logMessage(err);
            success = false;
        }
    }
    else
    {
        QString err = "Login error - supplied certificate file " + m_cert_file + " does not exist!";
        logMessage(err);
        success = false;
    }
    QString keyfile(m_key_file);
    QFile kfile(keyfile);
    if(kfile.exists())
    {
        if(!kfile.open(QIODevice::ReadOnly))
        {
            QString err = "Login error - unable to open key file " + m_key_file;
            logMessage(err);
            success = false;
        }
    }
    else
    {
        QString err = "Login error - supplied key file " + m_key_file + " does not exist!";
        logMessage(err);
        success = false;
    }

    if (success)
    {
        QByteArray certdata = file.readAll();
        QByteArray keydata = kfile.readAll();

        QSslKey pkey(keydata,QSsl::Rsa);
        if(pkey.isNull())
        {
            logMessage("Login error - The key file has no content!");
        }
        QSslCertificate sslcert(certdata, QSsl::Pem);
        if(sslcert.isNull())
        {
            logMessage("Login error - The certificate file has no content!");
        }

        QNetworkRequest request;
        m_ssl_config.setLocalCertificate(sslcert);
        m_ssl_config.setPrivateKey(pkey);
        m_ssl_config.setProtocol(QSsl::TlsV1_2);
        request.setSslConfiguration(m_ssl_config);
        request.setUrl(loginurl);
        request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
        request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

        m_netmanager->post(request,payload);
    }
    return success;
}

//====================================================================
void MarketManager::getAccountFunds()
{
    QString url = "https://api.betfair.com/exchange/account/rest/v1.0/getAccountFunds/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    //request.setRawHeader(QByteArray("Accept-Encoding"),QByteArray("gzip, deflate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString qpayload("{\"wallet\":\"UK\"}");
    std::string bytes = qpayload.toStdString();
    //qDebug() << qpayload;
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> getAccountFunds REQUEST send.");
    m_netmanager->post(request,payload);

}

/*
//====================================================================
void MarketManager::getEventTypes()
{
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listEventTypes/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QByteArray payload("{\"filter\":{ }}");

    m_netmanager->post(request,payload);
}
*/

/*
//====================================================================
void MarketManager::getEventsForEventID(const QString& event_id)
{
    // This request gets sub events for the supplied event ID argument
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listEvents/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString qpayload("{\"filter\":{\"eventIds\": [");
    qpayload.append(event_id);
    qpayload.append(QString("]}}"));
    std::string bytes = qpayload.toStdString();
    //qDebug() << qpayload;
    QByteArray payload(bytes.c_str(),bytes.length());

    m_netmanager->post(request,payload);
}
*/
/*
//====================================================================
void MarketManager::getEventsListForEventTypeID(int event_type_id)
{
    // This request gets sub events for the supplied event ID argument
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listEvents/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString qpayload("{\"filter\":{\"eventTypeIds\": [");
    qpayload.append(QString::number(event_type_id));
    qpayload.append(QString("]}}"));
    std::string bytes = qpayload.toStdString();
    //qDebug() << qpayload;
    QByteArray payload(bytes.c_str(),bytes.length());

    m_netmanager->post(request,payload);
}
*/
/*
//====================================================================
void MarketManager::getMarketsForEventID(const QString& event_id, int max_market_count)
{
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listMarketCatalogue/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    // This is messy!

    //var requestFilters = '{"filter":{"eventTypeIds": [' + eventId + '],
    //"marketCountries":["GB"],"marketTypeCodes":["WIN"],
    //"marketStartTime":{"from":"'+jsonDate+'"}},
    //"sort":"FIRST_TO_START","maxResults":"'+race_count+'",
    //"marketProjection":["MARKET_DESCRIPTION","RUNNER_DESCRIPTION","MARKET_START_TIME","EVENT","COMPETITION"]}}';


    QString qpayload("{\"filter\":{\"eventIds\": [");
    qpayload.append(event_id);
    qpayload.append(QString("]}"));
    qpayload.append(QString(",\"sort\":\"FIRST_TO_START\",\"maxResults\":\""));
    qpayload.append(QString::number(max_market_count));
    qpayload.append(QString("\",\"marketProjection\":[\"MARKET_DESCRIPTION\",\"RUNNER_DESCRIPTION\",\"MARKET_START_TIME\",\"EVENT\",\"COMPETITION\"]}"));
    std::string bytes = qpayload.toStdString();
    //qDebug() << qpayload;
    QByteArray payload(bytes.c_str(),bytes.length());

    m_netmanager->post(request,payload);
}
*/

//====================================================================
void MarketManager::makeGenericListMarketCatalogueRequest(const QString& market_filter)
{
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listMarketCatalogue/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    // Use supplied filter to make the request.

    std::string bytes = market_filter.toStdString();
    logAPIMsg("----->>>>> listMarketCatalogue REQUEST : " + market_filter);
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> listMarketCatalogue REQUEST send.");
    m_netmanager->post(request,payload);
}

//====================================================================
void MarketManager::getTodaysUKHorseMarkets(int day_offset)
{

    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listMarketCatalogue/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    std::vector<QString> v_countries;
    v_countries.push_back("GB");
    std::vector<QString> v_types;
    v_types.push_back("WIN");
    QString evtypeid = "7";
    QString qpayload = betfair::utils::generateGenericLMCatFilter(v_countries,
                                                                  v_types,
                                                                  evtypeid,
                                                                  100,
                                                                  false,
                                                                  true);


    std::string bytes = qpayload.toStdString();
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> listMarketCatalogue REQUEST send.");
    m_netmanager->post(request,payload);
}

//====================================================================
void MarketManager::layFieldAt(const double& odds,
                               const double& stake,
                               bool b_persist_after_market_inplay)
{
    if (m_market.valid())
    {
        std::vector<std::int64_t> v_selection_ids;
        QString market_id = m_market.getMarketID();
        auto runners = m_market.getRunners();
        for (auto it = runners.begin(); it != runners.end(); ++it)
        {
            if ((*it)->isActive())
            {
                std::int64_t sel_id = (*it)->getID();
                v_selection_ids.push_back(sel_id);
            }
        }
        //placeBets(market_id, v_selection_ids, odds, stake, true, b_persist_after_market_inplay);
    }
}

//====================================================================
void MarketManager::retrieveUnmatchedBetsForMarket(const QString& market_id)
{
    retrieveBetsForMarket(market_id,"EXECUTABLE");
}

//====================================================================
void MarketManager::retrieveMatchedBetsForMarket(const QString& market_id)
{
    retrieveBetsForMarket(market_id,"EXECUTION_COMPLETE");
}

//====================================================================
void MarketManager::retrieveAllBetsForMarket(const QString& market_id)
{
    retrieveBetsForMarket(market_id,"ALL");
}

//====================================================================
void MarketManager::retrieveBetsForMarket(const QString& market_id,
                                          const QString& order_proj)
{
    // Request
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listCurrentOrders/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");


    QString qpayload = QString("{\"marketIds\": [\"") + market_id + QString("\"],");
    qpayload.append(QString("\"orderProjection\":\""));
    qpayload.append(order_proj);
    qpayload.append(QString("\",\"dateRange\":{}}"));
    std::string bytes = qpayload.toStdString();
    //qDebug() << qpayload;
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> listCurrentOrders REQUEST send.");
    m_netmanager->post(request,payload);
}

//====================================================================
void MarketManager::cancelBets(const QString& market_id,
                               const std::vector<QString>& v_bets)
{
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/cancelOrders/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString qpayload("{\"marketId\": \"");
    qpayload.append(market_id);
    qpayload.append(QString("\""));
    if (!v_bets.empty())
    {
        qpayload.append(QString(",\"instructions\":["));
        int i_ct = 0;
        for (const QString& i : v_bets)
        {
            QString cm = (i_ct > 0) ? "," : "";
            QString pi = "{\"betId\":\"" + i + "\"}";
            qpayload.append(cm + pi);
            ++i_ct;
        }
        qpayload.append(QString("]"));
    }
    qpayload.append(QString("}"));
    std::string bytes = qpayload.toStdString();
    logMessage("CANCEL BETS Request : " + qpayload);
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> cancelOrders REQUEST send.");
    m_netmanager->post(request,payload);
}

//====================================================================
void MarketManager::getMarketProfitAndLoss(const QString& market_id)
{
    // Request
    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listMarketProfitAndLoss/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString qpayload = QString("{\"marketIds\": [\"") + (market_id) + QString("\"]}");
    std::string bytes = qpayload.toStdString();
    //qDebug() << qpayload;
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> listMarketProfitAndLoss REQUEST send.");
    m_netmanager->post(request,payload);
}

//====================================================================
void MarketManager::placeBets(const QString& market_id,
                              const std::vector<betfair::utils::betInstruction>& bets,
                              const QString& strategy_reference_string)
{
    if (!bets.empty() && (bets.size() < 200))
    {
        QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/placeOrders/";
        QNetworkRequest request;
        request.setSslConfiguration(m_ssl_config);
        request.setUrl(url);
        request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
        request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
        request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
        request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");


        QJsonObject order_info;
        order_info.insert("marketId",market_id);
        QJsonArray instructions;
        for (auto bet : bets)
        {
            // Create orders now from bet instruction array.
            if (bet.market_id == market_id)
            {
                QJsonObject new_bet;
                new_bet.insert("selectionId",static_cast<qint64>(bet.selection));
                QString side = bet.b_lay_type ? "LAY" : "BACK";
                new_bet.insert("side",side);
                new_bet.insert("orderType","LIMIT");
                if (bet.customer_ref_string.isEmpty() == false)
                {
                    new_bet.insert("customerOrderRef",bet.customer_ref_string);
                }

                QJsonObject limit_order;
                limit_order.insert("size",bet.stake);
                limit_order.insert("price",bet.odds);
                QString pers = bet.b_persistence_flag ? "PERSIST" : "LAPSE";
                limit_order.insert("persistenceType",pers);


                new_bet.insert("limitOrder",limit_order);

                instructions.append(new_bet);
            }
            else
            {
                // error - individual bet market ID does not match the input - we should
                // log this error and ignore the bet.
            }
        }
        order_info.insert("instructions",instructions);
        if (strategy_reference_string.isEmpty() == false)
        {
            order_info.insert("customerStrategyRef",strategy_reference_string);
        }


        /*
        QString qpayload("{\"marketId\": \"");
        qpayload.append(market_id);
        qpayload.append(QString("\",\"instructions\":["));
        int i_ct = 0;

        for (auto bet : bets)
        {
            // Create orders now from bet instruction array.
            if (bet.market_id == market_id)
            {
                QString str_odds = QString::number(bet.odds,'f' ,2);
                QString str_stake = QString::number(bet.stake,'f',2);
                QString type = bet.b_lay_type ? "LAY" : "BACK";
                QString pers = bet.b_persistence_flag ? "PERSIST" : "LAPSE";
                QString cm = (i_ct > 0) ? "," : "";
                QString pi = "{\"selectionId\":\"" + QString::number(bet.selection) + "\",\"side\":\"" + type + "\",\"orderType\":\"LIMIT\",\"limitOrder\":";
                QString lo = "{\"size\":\"" + str_stake + "\",\"price\":\"" + str_odds + "\",\"persistenceType\":\"" + pers + "\"}";
                if (bet.strategy_string.isEmpty() == false)
                {
                    lo.append(",\"customerOrderRef\":\"");
                    lo.append(bet.strategy_string);
                    lo.append("\"");
                }
                lo.append("}");
                qpayload.append(cm + pi + lo);
                ++i_ct;
            }
            else
            {
                // error - individual bet market ID does not match the input - we should
                // log this error and ignore the bet.
            }
        }
        qpayload.append("]");
        if (strategy_reference_string.isEmpty() == false)
        {
            qpayload.append(",\"customerStrategyRef\":\"");
            qpayload.append(strategy_reference_string);
            qpayload.append("\"");

        }
        qpayload.append(QString("}"));
        std::string bytes = qpayload.toStdString();
        //qDebug() << qpayload;
        */

        QJsonDocument doc(order_info);

        //QByteArray payload(bytes.c_str(),bytes.length());

        QByteArray payload = doc.toJson();
        m_netmanager->post(request,payload);


        logMessage("PLACE BETS Request : " + QString::fromUtf8(payload));
        logAPIMsg("----->>>>> placeOrders REQUEST send.");

    }

}


//====================================================================
void MarketManager::getMarketBook(const QString& market_id)
{
    // Get information on a single market!

    QString url = "https://api.betfair.com/exchange/betting/rest/v1.0/listMarketBook/";
    QNetworkRequest request;
    request.setSslConfiguration(m_ssl_config);
    request.setUrl(url);
    request.setRawHeader(QByteArray("X-Application"),QByteArray(m_app_key.toUtf8()));
    request.setRawHeader(QByteArray("X-Authentication"),QByteArray(m_session_token.toUtf8()));
    request.setRawHeader(QByteArray("accept"),QByteArray("application/json"));
    request.setRawHeader(QByteArray("Connection"),QByteArray("keep-alive"));
    //request.setRawHeader(QByteArray("Accept-Encoding"),QByteArray("gzip, deflate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QString qpayload("{\"marketIds\": [\"");
    qpayload.append(market_id);
    qpayload.append(QString("\"],\"priceProjection\":{\"priceData\":[\"EX_ALL_OFFERS\",\"EX_TRADED\"]}}"));
    //qpayload.append(QString(",\"priceData\":[\"EX_ALL_OFFERS\",\"EX_TRADED\"],\"virtualise\":false}}"));
    std::string bytes = qpayload.toStdString();
    QByteArray payload(bytes.c_str(),bytes.length());

    logAPIMsg("----->>>>> listMarketBook REQUEST send.");
    m_netmanager->post(request,payload);
}

//====================================================================
void MarketManager::parseLoginResponse(const QByteArray& response_data)
{
    m_error_string = "";
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI        
        m_error_string = "parseLoginResponse() error : " + jerror.errorString();
        return;
    }
    else
    {
        // no parser errors
        QJsonObject obj = jdoc.object();
        if (obj.value("loginStatus").toString() == "SUCCESS")
        {
            m_session_token = obj.value("sessionToken").toString();
            m_error_string = "SUCCESS";
            m_b_logged_in = true;
        }
        else
        {
            // report error
            m_error_string = "Error with logon: loginStatus=";
            m_error_string.append(obj.value("loginStatus").toString());
            m_error_string.append(".");
            m_b_logged_in = false;
        }
    }
    emit receiveLoginResponse();
}

//====================================================================
void MarketManager::parseCancelOrdersResponse(const QByteArray& response_data)
{
    m_error_string = "";
    logMessage("CANCEL BETS Response : " + QString::fromUtf8(response_data));
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI
        m_error_string = "parseCancelOrdersResponse() error : " + jerror.errorString();
    }
    else
    {
        QJsonObject obj = jdoc.object();
        QJsonArray instructions = obj.value("instructionReports").toArray();
        QString status = obj.value("status").toString();
        // Status should be SUCCESS,FAILURE or TIMEOUT
        QString error_code = obj.value("errorCode").toString();
        m_error_string = "CancelOrders response received. Status = " + status + ", Error code = " + error_code + "\n";
        QString mk_id = obj.value("marketId").toString();
        if (mk_id == m_market.getMarketID())
        {
            foreach (const QJsonValue& val, instructions)
            {
                QJsonObject current_item = val.toObject();
                QJsonObject instruction = current_item.value("instruction").toObject();
                QString instr_status = current_item.value("status").toString();
                QString instr_ec = current_item.value("errorCode").toString();

                QString betid = instruction.value("betId").toString();
                double szcanc = current_item.value("sizeCancelled").toDouble();
                QString cdate = current_item.value("cancelledDate").toString();
                if (instr_status == "SUCCESS")
                {
                    m_error_string += ("\n  Bet ID " + betid + ", status = " + instr_status + ", error code = " + instr_ec);
                    m_error_string += (", Amount cancelled = £" + QString::number(szcanc) + " at " + cdate);
                    std::shared_ptr<betfair::TBet> bt = m_market.getBetRecordByBetID(betid);
                    if (bt)
                    {
                        if (false == bt->isBetFullyMatched())
                        {
                            // Bet not fully matched which means it will be dealt with
                            // properly by the next listCurrentOrders response. If no volume was matched AT ALL
                            // then the bet will be completely missing from listCurrentOrders and so we must update
                            // the cancelled volume and set the cancel date HERE so that when the bet processed by
                            // the TBetfairMarket class, either the match will be PARTIAL in which case the bet is
                            // moved to the matched list (EXECUTION_COMPLETE) OR the bet is completely removed after
                            // being logged. If we dont update the cancellation date and volume below, when finally
                            // logged, the cancellation date will be empty and the cancelled volume will be ZERO.
                            // We just do this for logging completeness.
                            bt->setCancellationDate(cdate);
                            bt->updateCancelledVolume(szcanc);
                        }
                    }
                    else
                    {
                        m_error_string += ("\n  Unable to find Bet ID " + betid);
                    }
                }
                else
                {
                    m_error_string += ("\nCancellaton of bet ID " + betid + " FAILED!! Status = " + instr_status + ", error code = " + instr_ec);
                    m_error_string += (", Amount cancelled = £" + QString::number(szcanc) + " at " + cdate);
                }
            }
        }
        else
        {
            m_error_string += "\nReturned market ID does not match current selected market ID";
        }
    }
    if (!m_error_string.isEmpty())
    {
        // Log the cancellation report
        logMessage(m_error_string);
    }
    emit receiveCancelOrdersResponse();
}

//====================================================================
void MarketManager::parsePlaceOrdersResponse(const QByteArray& response_data)
{
    m_error_string = "";
    logMessage("PLACE BETS Response : " + QString::fromUtf8(response_data));
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI
        m_error_string += "parsePlaceOrdersResponse() error : JSON parse error " + jerror.errorString();
    }
    else
    {
        QJsonObject obj = jdoc.object();
        QJsonArray instructions = obj.value("instructionReports").toArray();
        QString status = obj.value("status").toString();
        bool b_bet_fail = (status != "SUCCESS");
        // Status should be SUCCESS,FAILURE or TIMEOUT
        QString error_code = obj.value("errorCode").toString();

        QString mk_id = obj.value("marketId").toString();
        foreach (const QJsonValue& bet, instructions)
        {
            QJsonObject current_bet = bet.toObject();
            const QString bet_stat = current_bet.value("status").toString();
            const QString bet_err = error_code + " : " + current_bet.value("errorCode").toString();
            if (bet_stat == "SUCCESS")
            {
                QJsonObject c_instr = current_bet.value("instruction").toObject();
                QJsonObject limit_order = c_instr.value("limitOrder").toObject();
                const double b_size = limit_order.value("size").toDouble();
                const double b_odds = limit_order.value("price").toDouble();
                const QString b_pers = limit_order.value("persistenceType").toString();

                const std::int64_t bet_sel_id = c_instr.value("selectionId").toInt();
                const QString bet_type = c_instr.value("side").toString();
                const bool b_lay_type = (bet_type == "LAY");


                const QString match_status = current_bet.value("orderStatus").toString();

                const QString bet_id = current_bet.value("betId").toString();
                const QString bet_place_date = current_bet.value("placedDate").toString();
                const double apm = current_bet.value("averagePriceMatched").toDouble();
                const double matched = current_bet.value("sizeMatched").toDouble();
                const double unmatched =  b_size - matched;

                // Create new record, and append to the market bet record list
                // First find the market                
                if (mk_id == m_market.getMarketID())
                {

                    std::shared_ptr<betfair::TBet> new_record = std::make_shared<betfair::TBet>(mk_id,
                                                                                                  bet_id,
                                                                                                  b_pers,
                                                                                                  bet_place_date,
                                                                                                  bet_stat,
                                                                                                  match_status,
                                                                                                  bet_err,
                                                                                                  bet_sel_id,
                                                                                                  b_odds,
                                                                                                  b_size,
                                                                                                  matched,
                                                                                                  unmatched,
                                                                                                  apm,
                                                                                                  b_lay_type);

                    QString error_info = "";
                    if (false == m_market.addPlacedBet(new_record, error_info))
                    {
                        m_error_string += (error_info + "\n");
                    }
                }
            }
            else
            {
                // The placement of the bet failed!
                m_error_string += "Failed to place bet! Response status: " + status + ", instruction status: " + bet_stat + ", error: " + bet_err + "\n";
            }
        }

        if (b_bet_fail)
        {
            m_error_string += "Failed to place bet! Status : " + status + ", error : " + error_code + "\n";
        }
    }
    if (!m_error_string.isEmpty())
    {
        logMessage(m_error_string);
    }
    emit receivePlaceBetsResponse();
}

//====================================================================
void MarketManager::parseListCurrentOrdersResponse(const QByteArray& response_data)
{
    m_error_string = "";
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    betfair::utils::logMessage(current_orders_log,"LIST CURRENT ORDERS Response : " + QString::fromUtf8(response_data));
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI        
        m_error_string = "parseListCurrentOrdersResponse() error : " + jerror.errorString();
    }
    else
    {
        // TODO: REMOVE ANY BETS IN MARKET THAT ARE NOT PRESENT IN THIS LIST!!!!
        std::set<QString> market_bets;
        QJsonObject obj = jdoc.object();
        QJsonArray orders_list = obj.value("currentOrders").toArray();
        bool more_avail = obj.value("moreAvailable").toBool();
        foreach (const QJsonValue& curr_order, orders_list)
        {
            QJsonObject order = curr_order.toObject();
            const QString mk_id = order.value("marketId").toString();
            if (m_market.getMarketID() == mk_id)
            {
                const QString bet_id = order.value("betId").toString();
                QString order_status = order.value("status").toString();
                const double match_size = order.value("sizeMatched").toDouble(-1.0);
                const double avgprice = order.value("averagePriceMatched").toDouble(-1.0);
                const double unmatched = order.value("sizeRemaining").toDouble(-1.0);
                const double lapsed = order.value("sizeLapsed").toDouble(-1.0);
                const double cancelled = order.value("sizeCancelled").toDouble(-1.0);
                const double voided = order.value("sizeVoided").toDouble(-1.0);
                QJsonObject price_size = order.value("priceSize").toObject();
                const double size = price_size.value("size").toDouble();
                const double odds = price_size.value("price").toDouble();

                const QString bet_type = order.value("side").toString();
                const bool b_lay_type = (bet_type == "LAY");
                const std::int64_t bet_sel_id = order.value("selectionId").toInt();
                const QString b_pers = order.value("persistenceType").toString();
                const QString bet_place_date = order.value("placedDate").toString();

                market_bets.insert(bet_id);
                std::shared_ptr<betfair::TBet> cbet = m_market.getBetRecordByBetID(bet_id);
                if (cbet)
                {
                    if (false == cbet->isBetFullyMatched())
                    {
                        if (match_size > -1.0)
                        {
                            cbet->updateMatchedPortion(match_size);
                        }
                        if (avgprice > -1.0)
                        {
                            cbet->updateAveragePriceMatched(avgprice);
                        }
                        if (unmatched > -1.0)
                        {
                            cbet->updateUnmatchedPortion(unmatched);
                        }
                        if (lapsed > -1.0)
                        {
                            cbet->updateLapsedVolume(lapsed);
                        }
                        if (cancelled > -1.0)
                        {
                            cbet->updateCancelledVolume(cancelled);
                        }
                        if (voided > -1.0)
                        {
                            cbet->updateVoidedVolume(voided);
                        }
                        cbet->updateMatchedStatus(order_status);
                    }
                }
                else
                {                  
                    // We do not have the bet in memory, but the bet exists!
                    // we should add this as it must have been placed via another
                    // means                                                     
                    // Create new record, and append to the market bet record list
                    std::shared_ptr<betfair::TBet> new_record = std::make_shared<betfair::TBet>(mk_id,
                                                                                                bet_id,
                                                                                                b_pers,
                                                                                                bet_place_date,
                                                                                                "SUCCESS",
                                                                                                order_status,
                                                                                                "",
                                                                                                bet_sel_id,
                                                                                                odds,
                                                                                                size,
                                                                                                match_size,
                                                                                                unmatched,
                                                                                                avgprice,
                                                                                                b_lay_type);

                    if (match_size > -1.0)
                    {
                        new_record->updateMatchedPortion(match_size);
                    }
                    if (avgprice > -1.0)
                    {
                        new_record->updateAveragePriceMatched(avgprice);
                    }
                    if (unmatched > -1.0)
                    {
                        new_record->updateUnmatchedPortion(unmatched);
                    }
                    if (lapsed > -1.0)
                    {
                        new_record->updateLapsedVolume(lapsed);
                    }
                    if (cancelled > -1.0)
                    {
                        new_record->updateCancelledVolume(cancelled);
                    }
                    if (voided > -1.0)
                    {
                        new_record->updateVoidedVolume(voided);
                    }
                    QString error_info = "";
                    if (false == m_market.addPlacedBet(new_record, error_info))
                    {
                        m_error_string += (error_info + "\n");
                    }
                }
            }
        }
        // Find bets in the market object that were NOT returned by this response!
        QString elinfo = "";
        m_market.removeLapsedBets(market_bets, elinfo);
        if (elinfo.length() > 0)
        {
            m_error_string += (elinfo + "\n");
        }
    }
    emit receiveListCurrentOrdersResponse();
}

//====================================================================
void MarketManager::parseListMarketCatalogueResponse(const QByteArray& response_data)
{
    m_error_string = "";
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI        
        m_error_string = "parseListMarketCatalogueResponse() error : " + jerror.errorString();
    }
    else
    {
        // This is the response from a listMarketCatalogue attempt.
        QJsonArray jsonArray = jdoc.array();
        m_market_info_list.clear();
        QString estr = "";
        if (false == betfair::json_utils::qt::extractDetailedMarketInfo(response_data,m_market_info_list,estr))
        {
            m_market_info_list.clear();
            qDebug() << estr;
        }
        else
        {
            std::sort(m_market_info_list.begin(),m_market_info_list.end(),sortByStartTime());
        }
    }
    emit receiveListMarketCatalogueResponse();
}

//====================================================================
void MarketManager::parseListMarketBookResponse(const QByteArray& response_data)
{
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI        
        m_error_string = "parseListMarketBookResponse() error : " + jerror.errorString();
    }
    else
    {
        m_error_string = "";
        if (betfair::json_utils::qt::parseMarketSnapshot(response_data,m_market,m_error_string))
        {
            m_market.sortRunners();
            //m_market.sortRunnersByTotalMatchedVolume();
        }
        if (m_error_string.length() > 0)
        {
            m_gui_error_log_messages.push_back(m_error_string);
            logMessage(m_error_string);
        }
    }
    emit receiveListMarketBookResponse();
}

//====================================================================
void MarketManager::parseListMarketPAndLResponse(const QByteArray& response_data)
{
    m_error_string = "";
    QJsonParseError jerror;
    QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
    if (jerror.error != QJsonParseError::NoError)
    {
        // Get error string and store for GUI
        m_error_string = "parseListMarketPAndLResponse() error : " + jerror.errorString();
    }
    else
    {
        // This is the response from a listMarketCatalogue attempt.
        QJsonArray jsonArray = jdoc.array();
        foreach (const QJsonValue& curr_mkt, jsonArray)
        {
            QJsonObject mkt = curr_mkt.toObject();
            QString mk_id = mkt.value("marketId").toString();
            if (mk_id == m_market.getMarketID())
            {
                // This is our market
                QJsonArray pandl = mkt.value("profitAndLosses").toArray();
                foreach (const QJsonValue& runner, pandl)
                {
                    QJsonObject sel = runner.toObject();
                    std::int64_t selid = sel.value("selectionId").toInt();                    
                    double profit = sel.value("ifWin").toDouble(NAN);
                    m_market.updateRunnerProfitIfWins(selid,profit);
                }
                break;
            }
        }
    }
    emit receiveListMarketPAndLResponse();
}

//====================================================================
void MarketManager::replyFinished(QNetworkReply *reply)
{
    // Get the raw response JSON as a byte array.
    QByteArray response_data = reply->readAll();

    QNetworkReply::NetworkError nerr = reply->error();
    if (nerr == QNetworkReply::NoError)
    {        
        m_error_string = "";
        if ((reply->url().url()).contains("certlogin"))
        {
            logAPIMsg("<<<<<----- login RESPONSE received.");
            parseLoginResponse(response_data);
        }
        else if ((reply->url().url()).contains("getAccountFunds"))
        {
            logAPIMsg("<<<<<----- getAccountFunds RESPONSE received.");
            QJsonParseError jerror;
            QJsonDocument jdoc = QJsonDocument::fromJson(response_data,&jerror);
            if (jerror.error != QJsonParseError::NoError)
            {
                // Get error string and store for GUI
                m_error_string = jerror.errorString();               
            }
            else
            {
                QJsonObject obj = jdoc.object();
                m_account_balance = obj.value("availableToBetBalance").toDouble();
            }
            emit receiveBalanceResponse();
        }
        else if ((reply->url().url()).contains("listMarketCatalogue"))
        {
            logAPIMsg("<<<<<----- listMarketCatalogue RESPONSE received.");
            parseListMarketCatalogueResponse(response_data);
        }
        else if ((reply->url().url()).contains("listMarketBook"))
        {
            logAPIMsg("<<<<<----- listMarketBook RESPONSE received.");
            parseListMarketBookResponse(response_data);
        }
        else if ((reply->url().url()).contains("placeOrders"))
        {
            logAPIMsg("<<<<<----- placeOrders RESPONSE received.");
            parsePlaceOrdersResponse(response_data);
        }      
        else if ((reply->url().url()).contains("listCurrentOrders"))
        {
            logAPIMsg("<<<<<----- listCurrentOrders RESPONSE received.");
            parseListCurrentOrdersResponse(response_data);
        }
        else if ((reply->url().url()).contains("cancelOrders"))
        {
            logAPIMsg("<<<<<----- cancelOrders RESPONSE received.");
            parseCancelOrdersResponse(response_data);
        }
        else if ((reply->url().url()).contains("listMarketProfitAndLoss"))
        {
            logAPIMsg("<<<<<----- listMarketProfitAndLoss RESPONSE received.");
            parseListMarketPAndLResponse(response_data);
        }
    }
    else
    {
        if (m_log_responses_to_file)
        {
            QString error_report = " QNetworkReply error : " + reply->errorString() + "\n";
            QNetworkRequest req = reply->request();
            error_report.append("\tRequest URL: " + req.url().url() + "\n");
            error_report.append("\tRequest Headers:\n");

            QList<QByteArray> reqHeaders = req.rawHeaderList();
            foreach( QByteArray hn, reqHeaders )
            {
                QByteArray reqValue = req.rawHeader(hn);
                error_report.append(("\t\t" + hn + ": " + reqValue + "\n"));
            }

            error_report.append("\tResponse URL: " + reply->url().url() + "\n");
            error_report.append("\tResponse Headers:\n");
            QList<QByteArray> respHeaders = reply->rawHeaderList();
            foreach( QByteArray hn, reqHeaders )
            {
                QByteArray respValue = reply->rawHeader(hn);
                error_report.append(("t\t" + hn + ": " + respValue + "\n"));
            }

            QString resp = QString::fromUtf8(response_data);
            error_report.append("\tResponse Payload: " + resp + "\n");
            betfair::utils::logMessage(network_errors_log,error_report);
        }
        m_error_string = "QNetworkReply error: ";
        m_error_string.append(reply->errorString());
        emit receiveNetworkError();
    }
}

//====================================================================
void MarketManager::replySSLErrors( QNetworkReply * reply, const QList<QSslError> & errors )
{
    qDebug() << "SSLErrors found: " << reply->readAll();
    foreach( QSslError hn, errors )
    {
        qDebug() << hn.errorString();
    }

}

//====================================================================
void MarketManager::replyAuthenticationRequired (QNetworkReply * reply, QAuthenticator * authenticator)
{
    qDebug() << "User:" << authenticator->user() << ", Password = " << authenticator->password();
    qDebug() << "replyAuthenticationRequired(): " << reply->readAll();
}

//====================================================================
void MarketManager::replyProxyAuthenticationRequired (const QNetworkProxy & proxy, QAuthenticator * authenticator)
{
    qDebug() << "Network Hostname:" << proxy.hostName();
    qDebug() << "User:" << authenticator->user() << ", Password = " << authenticator->password();
    qDebug() << "replyProxyAuthenticationRequired(): ";
}

//====================================================================
void MarketManager::getGUIErrorMessages(std::vector<QString>& errors)
{
    errors.insert(errors.end(),m_gui_error_log_messages.begin(),m_gui_error_log_messages.end());
    m_gui_error_log_messages.clear();
}

//====================================================================
void MarketManager::logMessage(const QString& msg)
{

    m_logfile_message_buffer.push_back((QDateTime::currentDateTimeUtc().toString("[yyyyMMMdd HH:mm:ss.zzz] ") + msg + "\n"));
    if (m_logfile_message_buffer.size() > 10)
    {
        writeLogMessageBufferToFile();
    }
}

//====================================================================
void MarketManager::writeLogMessageBufferToFile()
{
    if (!m_logfile_message_buffer.empty())
    {
        QFile file(m_log_file);
        if (!file.open(QIODevice::Append | QIODevice::Text))
            return;

        QTextStream out(&file);
        for (auto it = m_logfile_message_buffer.begin(); it != m_logfile_message_buffer.end(); ++it)
        {
            out << *it;
        }
        file.close();
        m_logfile_message_buffer.clear();
        m_logfile_message_buffer.reserve(20);
    }
}

//====================================================================
void MarketManager::logAPIMsg(const QString& msg)
{
    if (m_log_responses_to_file)
    {
        m_api_message_buffer.push_back((QDateTime::currentDateTimeUtc().toString("[yyyyMMMdd HH:mm:ss.zzz] ") + msg + "\n"));
        if (m_api_message_buffer.size() > 50)
        {
            writeAPIMessageBufferToFile();
        }
    }
}
//====================================================================
void MarketManager::writeAPIMessageBufferToFile()
{
    if (!m_api_message_buffer.empty())
    {
        QFile file(api_log);
        if (file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&file);
            for (auto it = m_api_message_buffer.begin(); it != m_api_message_buffer.end(); ++it)
            {
                out << *it;
            }
            file.close();
            m_api_message_buffer.clear();
            m_api_message_buffer.reserve(50);
        }
    }
}
