#include "betviewmodel.h"
#include "betfair/bet.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <sstream>

const int num_bet_view_cols = 5;

//=================================================================
BetViewModel::BetViewModel(QObject *parent,
                           betfair::TBetfairMarket& mkt,
                           const std::uint16_t& disptheme)
    : QAbstractTableModel(parent),
      m_market(mkt),
      m_display_theme(disptheme)
{

}

//=================================================================
void BetViewModel::refresh()
{
    if (m_market.valid())
    {
        beginResetModel();
        endResetModel();
    }
}

//=================================================================
int BetViewModel::rowCount(const QModelIndex & /*parent*/) const
{
    int size = 0;
    if (m_market.valid())
    {
        size = static_cast<int>(m_market.getNumMatchedBets());
    }
    return size;
}

//=================================================================
int BetViewModel::columnCount(const QModelIndex & /*parent*/) const
{
    return num_bet_view_cols;
}

//=================================================================
QString BetViewModel::getSelectedMarketID() const
{
    return m_market.valid() ? m_market.getMarketID() : "";
}

//=================================================================
void BetViewModel::updateData()
{
    if (m_market.valid())
    {
        beginResetModel();
        endResetModel();
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex(static_cast<int>(m_market.getNumMatchedBets()),6);
        //emit a signal to make the view reread identified data
        emit dataChanged(topLeft, bottomRight);
    }
}

//=================================================================
QVariant BetViewModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (m_market.valid() && row >= 0 && col >= 0)
    {
        auto bets = m_market.getFullyMatchedBets();
        int num_bets = static_cast<int>(bets.size());
        QString display_val("");
        if (row < num_bets)
        {
            std::shared_ptr<betfair::TBet> this_bet = bets[static_cast<std::size_t>(row)];
            switch(role)
            {
                case Qt::DisplayRole:

                    if (0 == col)
                    {                        
                        display_val = this_bet->getSelectionName();
                    }
                    else if (1 == col)
                    {
                        display_val = "£" + QString::number(this_bet->getStake(),'f',2);
                    }
                    else if (2 == col)
                    {
                        display_val = QString::number(this_bet->getAskPrice(),'f',2);
                    }
                    else if (3 == col)
                    {
                        display_val = "£" + QString::number(this_bet->getMatchedPortion(),'f',2);
                    }
                    else if (4 == col)
                    {
                        display_val = QString::number(this_bet->getAvgMatchOdds(),'f',2);
                    }
                    return display_val;

                case Qt::FontRole:
                    /*
                    if (col ==  0)
                    {
                        QFont boldFont;
                        boldFont.setBold(true);
                        return boldFont;
                    }
                    */
                    break;
                case Qt::ForegroundRole:
                    if (row < num_bets)
                    {
                        if (col < 6)
                        {
                            bool vlc = this_bet->isVoidLapsedCancelled();
                            if (vlc)
                            {
                                return QVariant::fromValue(QColor(Qt::white));
                            }
                            else
                            {
                                return QVariant::fromValue(QColor(Qt::black));
                            }
                        }
                    }
                    break;
                case Qt::BackgroundRole:
                    if (row < num_bets)
                    {
                        if (col < 6)
                        {
                            bool laytype = this_bet->isLay();
                            bool matched = this_bet->isBetFullyMatched();
                            bool vlc = this_bet->isVoidLapsedCancelled();
                            if (laytype)
                            {

                                if (vlc)
                                {
                                    QBrush t(QColor::fromRgb(50, 50, 50, 100));
                                    return t;
                                }
                                if (matched)
                                {
                                    QBrush t(QColor::fromRgb(255, 51, 51, 95));
                                    return t;
                                }
                                else
                                {
                                    QBrush t(QColor::fromRgb(217, 140, 179, 95));
                                    return t;
                                }
                            }
                            else
                            {
                                if (vlc)
                                {
                                    QBrush t(QColor::fromRgb(50, 50, 50, 100));
                                    return t;
                                }
                                if (matched)
                                {
                                    QBrush t(QColor::fromRgb(51, 51, 255, 95));
                                    return t;
                                }
                                else
                                {
                                    QBrush t(QColor::fromRgb(102, 204, 255, 95));
                                    return t;
                                }
                            }
                        }
                    }
                    break;
                case Qt::TextAlignmentRole:

                    if (row >= 0 && col >= 0)
                    {
                        return Qt::AlignHCenter + Qt::AlignVCenter;
                    }
                    break;
            }
        }
    }
    return QVariant();
}

//=================================================================
QVariant BetViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
                case 0:
                    return QString("SELECTION");
                case 1:
                    return QString("STAKE");
                case 2:
                    return QString("ODDS");
                case 3:
                    return QString("MATCHED");
                case 4:
                    return QString("AVG ODDS");
                case 5:
                    return QString("");
                default:
                    return QString("");
            }
        }

    }
    return QVariant();
}
