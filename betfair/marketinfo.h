#ifndef MARKETINFO_H
#define MARKETINFO_H

#include <string>
#include <cstdint>
#include <map>
#include <QDateTime>
#include "runnermetainfo.h"

namespace betfair {

class TMarketInfo
{
private:
    QString m_id;
    QString m_name;
    QString m_details;
    QString m_event_name;
    QString m_venue;    
    QString m_event_type_id;
    QString m_event_type_name;
    QDateTime m_start_time;
    bool m_goes_inplay;

    std::map<std::int64_t, TRunnerMetaInfo> m_selections;

public:
    TMarketInfo();
    void reset();
    QString getMarketID() const {return m_id;}
    QString getMarketName() const {return m_name;}
    QString getMarketDetails() const {return m_details;}
    QString getEventName() const {return m_event_name;}
    QDateTime getStartTime() const {return m_start_time;}

    QString getVenue() const {return m_venue;}
    QString getLabel() const;
    QString getEventTypeID() const {return m_event_type_id;}
    QString getEventTypeName() const {return m_event_type_name;}

    bool goesInplay() const {return m_goes_inplay;}

    void setMarketID(const QString& id) {m_id = id;}
    void setMarketName(const QString& name) {m_name = name;}
    void setEventName(const QString& event_name) {m_event_name = event_name;}
    void setStartTime(const QDateTime& st_time) {m_start_time = st_time;}
    void setMarketDetails(const QString& info) {m_details = info;}
    void setVenue(const QString& ven) {m_venue = ven;}
    void setGoesInplayFlag(bool ip) {m_goes_inplay = ip;}
    void setEventTypeID(const QString& evtid) {m_event_type_id = evtid;}
    void setEventTypeName(const QString& evtn) {m_event_type_name = evtn;}

    bool insertSelection(const TRunnerMetaInfo& new_runner);

    bool selectionIDExists(std::int64_t id) const;
    TRunnerMetaInfo getSelectionMetaData(std::int64_t id) const;

    std::int64_t getSelectionIDFromName(const QString& name) const;

    const std::map<std::int64_t, TRunnerMetaInfo>& getAllMetaData() const {return m_selections;}

};


}; // end of namespace betfair

#endif // MARKETINFO_H
