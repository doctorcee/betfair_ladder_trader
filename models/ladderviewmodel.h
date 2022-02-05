#ifndef LADDERVIEWMODEL_H
#define LADDERVIEWMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include "betfair/betfairmarket.h"

namespace ladderview {

    enum {COLUMN_COUNT = 7};
    enum eLadderColumns {
        HEDGE = 0,
        LAY_BET = 1,
        LAY_AV = 2,
        ODDS = 3,
        BACK_AV = 4,
        BACK_BET = 5,
        TRADED = 6
    };

} // end of namespace ladderview

class LadderViewModel : public QAbstractTableModel
{
public:
    LadderViewModel(QObject *parent,
                    betfair::TBetfairMarket& mkt,
                    const std::uint16_t& disptheme);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void updateVisibleData(const QModelIndex& topLeft,
                            const QModelIndex& bottomRight);

    void updateAllData();
    void refresh();
    void setSelectedRunner(const std::int64_t& new_id);

    QString getSelectedMarketID() const;
    std::int64_t getSelectionID() const {return m_selection_id;}

    int getLastLPMRow() const;
    int getRowCount() const {return m_row_count;}
    int getColCount() const {return m_col_count;}
    std::uint16_t getTradeHistorySecondsLo() const {return m_history_seconds_lo;}
    std::uint16_t getTradeHistorySecondsHi() const {return m_history_seconds_hi;}
    void setTradeHistorySecondsHi(const std::uint16_t& val) {m_history_seconds_hi = val;}
    void setTradeHistorySecondsLo(const std::uint16_t& val) {m_history_seconds_lo = val;}


    void setDisplayTheme(const std::uint16_t& disptheme) {m_display_theme = disptheme;}
    std::uint16_t getDisplayTheme() const {return m_display_theme;}

    void setBettingEnabled(bool val) {m_betting_enabled = val;}
    bool isBettingEnabled() const {return m_betting_enabled;}

private:

    betfair::TBetfairMarket& m_market;
    std::int64_t m_selection_id;
    const std::vector<QString> m_odds_vector;
    const int m_row_count;
    const int m_col_count;
    std::uint16_t m_history_seconds_hi;
    std::uint16_t m_history_seconds_lo;
    std::uint16_t m_display_theme;
    mutable int m_last_lpm_row;

    QColor m_lay_money_colour[3];   // regular,flash,text
    QColor m_suspended_colour[2];   // regular,text
    QColor m_odds_colour[2];        // regular,text
    QColor m_odds_traffic_light_colours[3];
    QColor m_back_money_colour[3];  // regular,flash,text
    QColor m_hedge_colours[3];

    bool m_betting_enabled;

};

#endif // LADDERVIEWMODEL_H
