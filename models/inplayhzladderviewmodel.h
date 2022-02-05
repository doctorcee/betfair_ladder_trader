#ifndef INPLAYHORIZONTALLADDER_H
#define INPLAYHORIZONTALLADDER_H

#include <QAbstractTableModel>
#include "betfair/betfairmarket.h"
#include <QString>

namespace iphzladdview {

    enum {COLUMN_COUNT = 27};
    enum {NUM_PRICES = 20};
    enum eIPHZColumns {
        NAME = 0,
        PROFIT = 1,
        MIN = 2,
        MAX = 3,
        LPM = 4
    };

} // end of namespace iphzladdview

class InPlayHZLadderModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    InPlayHZLadderModel(QObject *parent,
                        betfair::TBetfairMarket& my_market_ref,
                        const QString& m_image_dir,
                        const std::uint16_t& disptheme);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;    

    void updateData();
    void refresh();

    QString getSelectedMarketID() const;
    double getColumnOdds(int col);
    void setDisplayTheme(const std::uint16_t& disptheme) {m_display_theme = disptheme;}
    std::uint16_t getDisplayTheme() const {return m_display_theme;}

    void setBettingEnabled(bool val) {m_betting_enabled = val;}
    bool isBettingEnabled() const {return m_betting_enabled;}

private:

    betfair::TBetfairMarket& m_bf_market;
    bool m_lay_mode;
    QString m_image_dir;
    std::uint16_t m_display_theme;

    QColor m_matched_price_colour[2];   // colour,text
    QColor m_active_colour[2];          // colour,text
    QColor m_inactive_color[2];         // colour,text
    QColor m_lay_all_colour[2];         // colour,text
    QColor m_suspended_colour[2];

    bool m_betting_enabled;
};

#endif // INPLAYHORIZONTALLADDER_H
