#include "inplayhzladderviewmodel.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <sstream>


static const double price_array[iphzladdview::NUM_PRICES] = {1.01,1.1,1.2,1.3,1.4,1.5,1.6,1.7,1.8,1.9,
                                                             2.0,2.2,2.4,2.6,2.8,3.0,3.5,4.0,5.0,6.0};

//=================================================================
InPlayHZLadderModel::InPlayHZLadderModel(QObject *parent,
                                         betfair::TBetfairMarket& my_market_ref,
                                         const QString& image_dir,
                                         const std::uint16_t& disptheme)
    : QAbstractTableModel(parent),
      m_bf_market(my_market_ref),
      m_lay_mode(true),
      m_image_dir(image_dir),
      m_display_theme(disptheme),
      m_nonrunner_background(m_display_theme == 0 ? betfair::utils::veryLightGrey1 : Qt::transparent),
      m_nonrunner_foreground(betfair::utils::midgrey1),
      m_matched_price_colour{betfair::utils::greenTwo,Qt::black},
      m_active_colour{betfair::utils::layOne,Qt::black},
      m_inactive_color{betfair::utils::midgrey1,Qt::black},         // colour,text
      m_lay_all_colour{QColor(0x60,0x11,0x00,0xff),Qt::white},         // colour,text
      m_suspended_colour{betfair::utils::susp,Qt::black},
      m_betting_enabled(false)
{
    if (1 == m_display_theme)
    {
        m_inactive_color[0] = QColor(0x53,0x53,0x53);
        m_inactive_color[1] = Qt::black;
    }
}

//=================================================================
int InPlayHZLadderModel::rowCount(const QModelIndex & /*parent*/) const
{
    int size = 0;
    if (m_bf_market.valid())
    {
        size = 1 + static_cast<int>(m_bf_market.getNumRunners());
    }
    return size;
}

//=================================================================
int InPlayHZLadderModel::columnCount(const QModelIndex & /*parent*/) const
{
    return iphzladdview::COLUMN_COUNT;
}

//=================================================================
QString InPlayHZLadderModel::getSelectedMarketID() const
{
    return (m_bf_market.valid() ? m_bf_market.getMarketID() : "");
}


//=================================================================
void InPlayHZLadderModel::refresh()
{
    if (m_bf_market.valid())
    {
        beginResetModel();
        endResetModel();
    }
}

//=================================================================
void InPlayHZLadderModel::updateData()
{
    if (m_bf_market.valid())
    {
        QModelIndex topLeft = createIndex(0,0);
        QModelIndex bottomRight = createIndex(static_cast<int>(m_bf_market.getNumRunners()),iphzladdview::COLUMN_COUNT);
        //emit a signal to make the view reread identified data
        emit dataChanged(topLeft, bottomRight);
    }
}

//=================================================================
double InPlayHZLadderModel::getColumnOdds(int col)
{
    double odds = 0.0;
    if ((col > iphzladdview::LPM) && (col < (iphzladdview::NUM_PRICES+5)))
    {
        odds = price_array[col-5];
    }
    return odds;
}

//=================================================================
QVariant InPlayHZLadderModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if (m_bf_market.valid() && col >= 0)
    {
        const bool b_suspended = (m_bf_market.getMarketStatus() == "SUSPENDED");
        const bool b_inplay = m_bf_market.marketInPlay();
        if (row == 0)
        {
            if ((col > iphzladdview::LPM) && (col < (iphzladdview::NUM_PRICES + 5)))
            {
                switch(role)
                {                    
                    case Qt::DisplayRole:
                        return QString("LAY\nALL");
                    case Qt::BackgroundRole:
                        if (m_bf_market.closed())
                        {
                            return QBrush(m_inactive_color[0]);
                        }
                        else
                        {
                            if (b_suspended)
                            {
                                return QBrush(betfair::utils::susp);
                            }
                            else
                            {
                                return b_inplay ? QBrush(m_lay_all_colour[0]) : QBrush(m_inactive_color[0]);
                            }
                        }
                    case Qt::ForegroundRole:
                        if (m_bf_market.closed())
                        {
                            return QBrush(m_inactive_color[1]);
                        }
                        else
                        {
                            return b_inplay ? QBrush(m_lay_all_colour[1]) : QBrush(m_inactive_color[1]);
                        }
                    case Qt::TextAlignmentRole:
                        return Qt::AlignHCenter + Qt::AlignVCenter;
                }
            }
        }
        else if (row > 0)
        {
            --row;
            QString display_val("");
            int num_runners = static_cast<int>(m_bf_market.getNumRunners());
            if (row < num_runners)
            {
                auto runners = m_bf_market.getRunners();
                const std::size_t runner_index = static_cast<std::size_t>(row);
                std::shared_ptr<betfair::TRunner> this_runner = runners[runner_index];
                const bool b_runner_active = this_runner->isActive();
                double imin(0.0),imax(0.0),lpm(0.0);
                switch(role)
                {
                    case Qt::DecorationRole:
                        if (iphzladdview::NAME == col)
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
                        this_runner->getLatestIntervalData(imin,imax,lpm);
                        if (iphzladdview::NAME == col)
                        {
                            display_val = "  " + this_runner->getName();
                            if (b_runner_active == false)
                            {
                                display_val.append(" (N/R)");
                            }
                            display_val.append("  ");
                        }
                        else
                        {
                            if (b_runner_active)
                            {
                                if (iphzladdview::PROFIT == col)
                                {

                                    double profitifwins = this_runner->getProfit();
                                    display_val = profitifwins < 0.0 ? "-£" : "£";
                                    display_val += QString::number(std::abs(profitifwins),'f',2);
                                }
                                else if (iphzladdview::MIN == col)
                                {
                                    if (imin > 1.0)
                                    {
                                        display_val = QString::number(imin,'f',2);
                                    }
                                }
                                else if (iphzladdview::MAX == col)
                                {
                                    if (imax > 1.0)
                                    {
                                        display_val = QString::number(imax,'f',2);
                                    }
                                }
                                else if (iphzladdview::LPM == col)
                                {
                                    if (lpm > 1.0)
                                    {
                                        display_val = QString::number(lpm,'f',2);
                                    }
                                }
                                else if (col < (iphzladdview::NUM_PRICES + 5))
                                {
                                    // These slots should contain amounts of unmatched bets
                                    // at this price
                                    int price_index = col - 5;
                                    const double this_price = price_array[price_index];
                                    display_val = ("  " + QString::number(this_price,'f',2) + "  ");
                                }
                            }
                        }
                        return display_val;
                    case Qt::FontRole:
                        if (col < iphzladdview::MIN)
                        {
                            QFont boldFont;
                            boldFont.setBold(true);
                            return boldFont;
                        }
                        break;
                    case Qt::ForegroundRole:
                        if (iphzladdview::NAME == col)
                        {                            
                            return b_runner_active ? QBrush(Qt::black) : QBrush(m_nonrunner_foreground);
                        }
                        else if (iphzladdview::PROFIT == col)
                        {                            
                            return QBrush(Qt::white);
                        }
                        else if ((col > iphzladdview::LPM) && (col < (iphzladdview::NUM_PRICES+5)))
                        {
                            if (b_suspended)
                            {
                                return QBrush(m_suspended_colour[1]);
                            }
                            else
                            {
                                return b_inplay ? QBrush(m_active_colour[1]) : QBrush(m_inactive_color[1]);
                            }
                        }
                        break;
                    case Qt::BackgroundRole:
                        this_runner->getLatestIntervalData(imin,imax,lpm);
                        if (iphzladdview::NAME == col)
                        {
                            return b_runner_active ? QBrush(Qt::white) : QBrush(m_nonrunner_background);
                        }
                        else if (iphzladdview::PROFIT == col)
                        {
                            if (b_runner_active)
                            {
                                if (this_runner->getProfit() > 0.0)
                                {
                                    QBrush t(betfair::utils::green1);
                                    return t;
                                }
                                else if (this_runner->getProfit() < 0.0)
                                {
                                    QBrush t(betfair::utils::redTwo);
                                    return t;
                                }
                                else
                                {
                                    QBrush t(betfair::utils::midgrey1);
                                    return t;
                                }
                            }
                        }
                        else if ((col > iphzladdview::PROFIT) && (col <= iphzladdview::LPM))
                        {

                        }
                        else if ((col > iphzladdview::LPM) && (col < (iphzladdview::NUM_PRICES+5)))
                        {
                            if (b_suspended)
                            {
                                return QBrush(betfair::utils::susp);
                            }
                            if (b_runner_active)
                            {
                                if (!b_inplay || !m_betting_enabled)
                                {
                                    return QBrush(m_inactive_color[0]);
                                }
                                else
                                {
                                    int price_index = col - 5;
                                    bool flash = false;
                                    const double upper = price_array[price_index];
                                    const double lower = (price_index > 0) ? price_array[price_index-1] : 1.0;
                                    if (imin > 1.0)
                                    {
                                        if (imin > lower && imin <= upper)
                                        {
                                            flash = true;
                                        }
                                        else
                                        {
                                            if (imax > lower && imax <= upper)
                                            {
                                                flash = true;
                                            }
                                            else
                                            {
                                                if (imin <= lower && imax >= upper)
                                                {
                                                    flash = true;
                                                }
                                            }
                                        }
                                    }
                                    if (flash)
                                    {
                                        return QBrush(m_matched_price_colour[0]);
                                    }
                                    else
                                    {
                                        return QBrush(m_active_colour[0]);
                                    }
                                }
                            }
                            else
                            {
                                return QBrush(m_nonrunner_background);
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
    }
    return QVariant();
}

//=================================================================
QVariant InPlayHZLadderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            if (iphzladdview::NAME == section)
            {
                return QString("   RUNNER   ");
            }
            else if (iphzladdview::PROFIT == section)
            {
                return QString("  PROFIT  ");
            }
            else if (iphzladdview::MIN == section)
            {
                return QString(" LAST MIN ");
            }
            else if (iphzladdview::MAX == section)
            {
                return QString(" LAST MAX ");
            }
            else if (iphzladdview::LPM == section)
            {
                return QString("     LPM     ");
            }
        }
        else if (orientation == Qt::Vertical)
        {
            return (QString(" ") + QString::number(section + 1) + QString(" "));
        }
    }
    return QVariant();
}


