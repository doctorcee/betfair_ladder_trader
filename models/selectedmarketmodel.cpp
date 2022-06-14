#include "selectedmarketmodel.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <sstream>

//=================================================================
SelectedMarketModel::SelectedMarketModel(QObject *parent,
                                         betfair::TBetfairMarket& my_market_ref,
                                         const QString& image_dir,
                                         const std::uint16_t& disptheme)
    : QAbstractTableModel(parent),
      m_bf_market(my_market_ref),
      m_image_dir(image_dir),
      m_display_theme(disptheme),
      m_nonrunner_background(m_display_theme == 0 ? betfair::utils::veryLightGrey1 : Qt::transparent),
      m_nonrunner_foreground(betfair::utils::midgrey1),
      m_betting_enabled(false)
{

}

//=================================================================
int SelectedMarketModel::rowCount(const QModelIndex & /*parent*/) const
{
    int size = 0;
    if (m_bf_market.valid())
    {
        size = static_cast<int>(m_bf_market.getNumRunners());
    }
    return size;
}

//=================================================================
int SelectedMarketModel::columnCount(const QModelIndex & /*parent*/) const
{
    return gridview::COLUMN_COUNT;
}

//=================================================================
QString SelectedMarketModel::getSelectedMarketID() const
{
    return (m_bf_market.valid() ? m_bf_market.getMarketID() : "");
}


//=================================================================
void SelectedMarketModel::refresh()
{
    if (m_bf_market.valid())
    {
        beginResetModel();
        endResetModel();
    }
}

//=================================================================
void SelectedMarketModel::updateData()
{
    // Dont update the FIRST column
    if (m_bf_market.valid())
    {
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex(static_cast<int>(m_bf_market.getNumRunners()),gridview::COLUMN_COUNT);
        //emit a signal to make the view reread identified data
        emit dataChanged(topLeft, bottomRight);
    }
}


//=================================================================
QVariant SelectedMarketModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (m_bf_market.valid() && row >= 0 && col >= gridview::IMAGE)
    {
        bool b_suspended = (m_bf_market.getMarketStatus() == "SUSPENDED");
        switch(role)
        {        
            case Qt::DecorationRole:
                if (gridview::NAME == col)
                {
                    int num_runners = static_cast<int>(m_bf_market.getNumRunners());
                    if (row < num_runners)
                    {
                        auto runners = m_bf_market.getRunners();
                        const std::size_t runner_index = static_cast<std::size_t>(row);
                        std::shared_ptr<betfair::TRunner> this_runner = runners[runner_index];
                        QString fname = m_image_dir + "/" + QString::number(this_runner->getID()) + ".jpg";
                        QPixmap pixmap(fname);
                        return pixmap;
                    }
                }
                break;


            case Qt::DisplayRole:           
                if (row < static_cast<int>(m_bf_market.getNumRunners()))
                {
                    QString display_val("");
                    auto runners = m_bf_market.getRunners();
                    const std::size_t runner_index = static_cast<std::size_t>(row);
                    std::shared_ptr<betfair::TRunner> this_runner = runners[runner_index];
                    bool b_runner_active = this_runner->isActive();
                    bool b_runner_won = this_runner->isWinner();
                    if (col == gridview::NAME)
                    {
                        display_val = this_runner->getName();
                        if (b_runner_active == false)
                        {
                            display_val.append(" N/R");
                        }
                        else
                        {                            
                            QString draw = this_runner->getStallDraw();
                            if (!draw.isEmpty())
                            {
                                display_val.append(" [" + draw + "]");
                            }
                        }
                        return display_val;
                    }
                    else if (col == gridview::PROFIT)
                    {
                        double profitifwins = this_runner->getProfit();
                        display_val = profitifwins < 0.0 ? "-£" : "£";
                        display_val += QString::number(std::abs(profitifwins),'f',2);
                        return display_val;
                    }
                    else if (col >= gridview::BACK3 && col <= gridview::BACK1)
                    {
                        if (b_runner_won)
                        {
                            display_val = "WINNER";
                        }
                        else
                        {
                            if (b_runner_active)
                            {
                                const int pindex = 5 - col;
                                std::pair<double,double> bp = this_runner->getOrderedBackPrice(pindex);
                                if (bp.first > 0.0)
                                {
                                    display_val = QString::number(bp.first,'f',2) + QString("\n£") + QString::number(bp.second,'f',2);
                                }
                            }
                        }
                        return display_val;
                    }
                    else if (col >= gridview::LAY1 && col <= gridview::LAY3)
                    {
                        if (b_runner_won)
                        {
                            display_val = "WINNER";
                        }
                        else
                        {
                            if (b_runner_active)
                            {
                                const int pindex = col-6;
                                std::pair<double,double> bp = this_runner->getOrderedLayPrice(pindex);
                                if (bp.first > 0.0)
                                {
                                    display_val = QString::number(bp.first,'f',2) + QString("\n£") + QString::number(bp.second,'f',2);
                                }                                
                            }
                        }                       
                        return display_val;
                    }
                    else if (gridview::HEDGE == col)
                    {
                        if (b_runner_active)
                        {
                            display_val = "HEDGE POSITION";
                        }
                        return display_val;
                    }
                }
                return QString("");
            case Qt::FontRole:
                if (col ==  gridview::LAY1 || col == gridview::BACK1 || col == gridview::NAME || col == gridview::PROFIT)
                {
                    QFont boldFont;
                    boldFont.setBold(true);
                    return boldFont;
                }
                break;
            case Qt::ForegroundRole:                
                if (row < static_cast<int>(m_bf_market.getNumRunners()))
                {
                    auto runners = m_bf_market.getRunners();
                    const std::size_t runner_index = static_cast<std::size_t>(row);
                    std::shared_ptr<betfair::TRunner> this_runner = runners[runner_index];
                    if (false == this_runner->isActive())
                    {
                        return QBrush(m_nonrunner_foreground);
                    }
                    else
                    {
                        if (col == gridview::NAME)
                        {
                            return QBrush(Qt::black);
                        }
                        else if (col >= gridview::BACK3 && col <= gridview::LAY3)
                        {
                            return QBrush(Qt::black);
                        }
                        else if (col == gridview::PROFIT)
                        {
                            if (this_runner->getProfit() > 0.0)
                            {
                                return (m_display_theme == 0) ? QBrush(betfair::utils::green1) : QBrush(Qt::white);
                            }
                            else if (this_runner->getProfit() < 0.0)
                            {
                                return (m_display_theme == 0) ? QBrush(betfair::utils::redTwo) : QBrush(Qt::white);
                            }
                            else
                            {
                                return (m_display_theme == 0) ? QBrush(Qt::black) : QBrush(Qt::white);
                            }
                        }
                        else if (col == gridview::HEDGE)
                        {
                            return QBrush(Qt::black);
                        }
                    }
                }
                return QBrush(Qt::transparent);
            case Qt::BackgroundRole:                
                if (row < static_cast<int>(m_bf_market.getNumRunners()))
                {
                    auto runners = m_bf_market.getRunners();
                    const std::size_t runner_index = static_cast<std::size_t>(row);
                    std::shared_ptr<betfair::TRunner> this_runner = runners[runner_index];
                    if (false == this_runner->isActive())
                    {
                        return QBrush(m_nonrunner_background);
                    }
                    switch (col) {
                        case gridview::NAME:
                            return QBrush(Qt::white);
                        case gridview::PROFIT:
                            if (this_runner->getProfit() > 0.0)
                            {
                                return (m_display_theme == 1) ? QBrush(betfair::utils::green1) : QBrush(Qt::white);
                            }
                            else if (this_runner->getProfit() < 0.0)
                            {
                                return (m_display_theme == 1) ? QBrush(betfair::utils::redTwo) : QBrush(Qt::white);
                            }
                            else
                            {
                                return (m_display_theme == 1) ? QBrush(betfair::utils::midgrey1) : QBrush(Qt::white);
                            }
                        case gridview::BACK3:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::backThree);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        case gridview::BACK2:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::backTwo);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        case gridview::BACK1:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::backOne);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        case gridview::LAY1:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::layOne);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        case gridview::LAY2:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::layTwo);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        case gridview::LAY3:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::layThree);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        case gridview::HEDGE:
                            if (m_betting_enabled)
                            {
                                return b_suspended ? QBrush(betfair::utils::susp) : QBrush(betfair::utils::green1);
                            }
                            else
                            {
                                return QBrush(betfair::utils::midgrey1);
                            }
                        default:
                            return QBrush(Qt::transparent);
                    }
                }
                return QBrush(Qt::transparent);
            case Qt::TextAlignmentRole:

                if (row >= 0 && col >= 0)
                {
                    return Qt::AlignHCenter + Qt::AlignVCenter;
                }
                break;
            default:
                break;
        }
    }
    return QVariant();
}

//=================================================================
QVariant SelectedMarketModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
                case gridview::NAME:
                    return QString("SELECTION");
                case gridview::PROFIT:
                    return QString("PROFIT");
                case gridview::BACK1:
                    return QString("BACK");
                case gridview::LAY1:
                    return QString("LAY");                             
                default:
                    break;
            }
        }
        else if (orientation == Qt::Vertical)
        {      
            return (QString(" ") + QString::number(section + 1) + QString(" "));
        }
    }
    return QVariant();
}
