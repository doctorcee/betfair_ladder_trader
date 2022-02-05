#include "marketinfo.h"
#include <sstream>

namespace betfair {

//=================================================
TMarketInfo::TMarketInfo()
    : m_id(""),
      m_name(""),
      m_details(""),
      m_event_name(""),
      m_venue(""),
      m_event_type_id(""),
      m_event_type_name(""),
      m_start_time(),
      m_goes_inplay(false)
{

}
//=================================================
void TMarketInfo::reset()
{
    m_id = "";
    m_name = "";
    m_details = "";
    m_event_name = "";
    m_start_time = QDateTime();
    m_venue = "";
    m_event_type_id = "";
    m_event_type_name = "";
    m_goes_inplay = false;
    m_selections.clear();
}

//=================================================
bool TMarketInfo::selectionIDExists(std::int64_t id) const
{
    return (m_selections.find(id) != m_selections.end());
}

//=================================================
TRunnerMetaInfo TMarketInfo::getSelectionMetaData(std::int64_t id) const
{
    auto it = m_selections.find(id);
    if (it != m_selections.end())
    {
        return it->second;
    }
    else
    {
        return TRunnerMetaInfo();
    }
}

//=================================================
std::int64_t TMarketInfo::getSelectionIDFromName(const QString& name) const
{
    std::int64_t id = -1;
    if (!name.isEmpty())
    {
        for (auto it = m_selections.begin(); it != m_selections.end(); ++it)
        {
            if (it->second.name == name)
            {
                id = it->first;
                break;
            }
        }
    }
    return id;

}

//=================================================
QString TMarketInfo::getLabel() const
{
    QString lbl = "";
    if (m_event_type_id == "7" || m_event_type_id == "4339")
    {
        // For dogs and horses incorporate venue and start time into label
        lbl.append(m_venue + " " + m_start_time.toString("HH:mm") + " " + m_name);
    }
    else
    {
        lbl.append(m_event_name + " (" + m_name + ") ");
    }
    return lbl;
}

//=================================================
bool TMarketInfo::insertSelection(const TRunnerMetaInfo& new_runner)
{
    bool b_inserted = false;
    if (m_selections.find(new_runner.id) == m_selections.end())
    {
        // Does not exist - insert
        std::pair<std::int64_t,TRunnerMetaInfo> new_pair = std::make_pair(new_runner.id,new_runner);
        m_selections.insert(new_pair);
        b_inserted = true;
    }
    return b_inserted;
}





} // end of namespace betfair
