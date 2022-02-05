#ifndef SELECTEDMARKETMODEL_H
#define SELECTEDMARKETMODEL_H


#include <QAbstractTableModel>
#include "betfair/betfairmarket.h"
#include <QString>

namespace gridview {

    enum {COLUMN_COUNT = 11};
    enum eGVColumns {
        IMAGE = 0,
        NAME = 1,
        PROFIT = 2,
        BACK3 = 3,
        BACK2 = 4,
        BACK1 = 5,
        LAY1 = 6,
        LAY2 = 7,
        LAY3 = 8,
        HEDGE = 9
    };

} // end of namespace gridview

class SelectedMarketModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    SelectedMarketModel(QObject *parent,
                        betfair::TBetfairMarket& my_market_ref,
                        const QString& image_dir,
                        const std::uint16_t& disptheme);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;    

    void updateData();
    void updateImageDataCol();
    void refresh();

    QString getSelectedMarketID() const;

    void setDisplayTheme(const std::uint16_t& disptheme) {m_display_theme = disptheme;}
    std::uint16_t getDisplayTheme() const {return m_display_theme;}

    void setBettingEnabled(bool val) {m_betting_enabled = val;}
    bool isBettingEnabled() const {return m_betting_enabled;}

private:

    betfair::TBetfairMarket& m_bf_market;
    QString m_image_dir;
    std::uint16_t m_display_theme;

    bool m_betting_enabled;


};

#endif // SELECTEDMARKETMODEL_H
