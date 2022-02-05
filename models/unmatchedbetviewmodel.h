#ifndef UNMATCHEDBETVIEWMODEL_H
#define UNMATCHEDBETVIEWMODEL_H

#include <QAbstractTableModel>
#include "betfair/betfairmarket.h"
#include <QString>

class UnmatchedBetViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    UnmatchedBetViewModel(QObject *parent,
                          betfair::TBetfairMarket& mkt,
                          const std::uint16_t& disptheme);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;    

    void updateData();
    void refresh();

    QString getSelectedMarketID() const;

    void setDisplayTheme(const std::uint16_t& disptheme) {m_display_theme = disptheme;}
    std::uint16_t getDisplayTheme() const {return m_display_theme;}

private:

    betfair::TBetfairMarket& m_market;
    std::uint16_t m_display_theme;
};


#endif // UNMATCHEDBETVIEWMODEL_H
