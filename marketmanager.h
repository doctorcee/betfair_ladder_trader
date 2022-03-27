//====================================================================
// Filename: marketmanager.h
// Version:
// Author:        Doctor C
//
// Created at:    12:00 2016/01/01
// Modified at:
// Description:   class to manage betfair API interaction and
//                market monitoring
//====================================================================


#ifndef MARKETMANAGER_H
#define MARKETMANAGER_H

#include <QObject>
#include <QNetworkReply>
#include "betfair/marketinfo.h"
#include "betfair/betfairmarket.h"
#include <memory>
#include <cstdint>
#include <map>


class MarketManager : public QObject
{
    Q_OBJECT
public:
    MarketManager(betfair::TBetfairMarket& mkt);
    ~MarketManager();
    void setLoginDetails(const QString& uname, const QString& passw,
                         const QString& certfile, const QString& keyfile,
                         const QString& appkey);

    //void login();
    bool attemptLogin();

    //void getEventTypes();
    //void getEventsListForEventTypeID(int event_type_id);
    //void getMarketsForEventID(const QString& event_id, int max_market_count);
    //void getEventsForEventID(const QString& event_id);
    void getMarketBook(const QString& market_id);
    void getAccountFunds();

    void cancelBets(const QString& market_id,
                    const std::vector<QString>& v_bets);

    void placeBets(const QString& market_id,
                   const std::vector<betfair::utils::betInstruction>& bets,
                   const QString& strategy_reference_string);

    void layFieldAt(const double& odds,
                    const double& stake,
                    bool b_persist_after_market_inplay);

    void retrieveAllBetsForMarket(const QString& market_id);
    void retrieveMatchedBetsForMarket(const QString& market_id);
    void retrieveUnmatchedBetsForMarket(const QString& market_id);
    void getMarketProfitAndLoss(const QString& market_id);

    void getTodaysUKHorseMarkets(int day_offset = 0);
    void makeGenericListMarketCatalogueRequest(const QString& market_filter);

    bool isLoggedIn() const;
    QString getErrorString() const;
    QString getSessionToken() const;
    QString getUname() const;
    const std::vector<betfair::TMarketInfo>& getMarketInfoList() const {return m_market_info_list;}

    betfair::TMarketInfo getMarketInfoByID(const QString& id);

    double getAvailableBalance() const {return m_account_balance;}
    void getGUIErrorMessages(std::vector<QString>& errors);

public slots:

    void replyFinished(QNetworkReply *reply);
    void replySSLErrors( QNetworkReply * reply, const QList<QSslError> & errors );
    void replyAuthenticationRequired (QNetworkReply * reply, QAuthenticator * authenticator);
    void replyProxyAuthenticationRequired (const QNetworkProxy & proxy, QAuthenticator * authenticator);



signals:
    void receiveNetworkError();
    void receiveLoginResponse();
    //void receiveListEventTypesResponse();
    //void receiveListEventsResponse();
    void receiveListMarketCatalogueResponse();
    void receiveListMarketBookResponse();
    void receiveBalanceResponse();
    void receivePlaceBetsResponse();
    void receiveListCurrentOrdersResponse();
    void receiveCancelOrdersResponse();
    void receiveListMarketPAndLResponse();

private:
    betfair::TBetfairMarket& m_market;
    QNetworkAccessManager* m_netmanager;
    std::vector<betfair::TMarketInfo> m_market_info_list;

    QSslConfiguration m_ssl_config;
    QString m_username;
    QString m_password;
    QString m_cert_file;
    QString m_key_file;
    QString m_app_key;
    QString m_session_token;
    QString m_error_string;
    std::vector<QString> m_logfile_message_buffer;
    std::vector<QString> m_api_message_buffer;
    std::vector<QString> m_gui_error_log_messages;

    QString m_log_file;
    QString m_error_file;

    double m_account_balance;
    bool m_b_logged_in;
    bool m_log_responses_to_file;

    void retrieveBetsForMarket(const QString& market_id, const QString& order_proj);
    void parseLoginResponse(const QByteArray& response_data);
    void parseListMarketCatalogueResponse(const QByteArray& response_data);
    void parseListMarketBookResponse(const QByteArray& response_data);
    void parsePlaceOrdersResponse(const QByteArray& response_data);
    void parseListCurrentOrdersResponse(const QByteArray& response_data);
    void parseCancelOrdersResponse(const QByteArray& response_data);
    void parseListMarketPAndLResponse(const QByteArray& response_data);

    void logMessage(const QString& msg);
    void writeLogMessageBufferToFile();

    void logAPIMsg(const QString& msg);
    void writeAPIMessageBufferToFile();



};

#endif // MARKETMANAGER_H
