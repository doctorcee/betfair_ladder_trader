#include "stakingboxmodel.h"
#include <QFont>
#include <QBrush>


//=================================================================
StakingBoxModel::StakingBoxModel(QObject *parent,
                                 int num_stakes,
                                 const std::uint16_t& disptheme)
    : QAbstractTableModel(parent),
      m_row_count(1),
      m_col_count(num_stakes),
      m_selected_column_index(0),
      m_base_stake(2),
      m_selected_stake(2),
      m_display_theme(disptheme)
{

}

//=================================================================
int StakingBoxModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_row_count;
}

//=================================================================
int StakingBoxModel::columnCount(const QModelIndex & /*parent*/) const
{
    return m_col_count;
}

//=================================================================
void StakingBoxModel::updateAllData()
{
    QModelIndex topLeft = createIndex(0,0);
    QModelIndex bottomRight = createIndex(m_row_count,m_col_count);
    emit dataChanged(topLeft, bottomRight);
}

//=================================================================
void StakingBoxModel::changeBaseStake(const std::uint32_t& new_stake)
{
    if (new_stake > 1 && new_stake < 24)
    {
        m_base_stake = new_stake;
        updateAllData();
    }
}

//=================================================================
void StakingBoxModel::setSelectedColumn(const int& col)
{
    if (col >= 0 && col < m_col_count)
    {
        m_selected_column_index = col;
        m_selected_stake = m_base_stake;
        int i = 0;
        while(i < m_selected_column_index)
        {
            m_selected_stake *= 2;
            ++i;
        }

    }
}

//=================================================================
QVariant StakingBoxModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    QString display_val = "";
    if (row == 0 && col >= 0 && col < m_col_count)
    {
        switch(role)
        {
            case Qt::DisplayRole:
                {
                std::uint32_t val = m_base_stake;
                int i = 0;
                while (i < col)
                {
                    val *= 2;
                    ++i;
                }
                display_val = QString::number(val);
                }
                return display_val;
            case Qt::FontRole:
                {
                QFont boldFont;
                boldFont.setBold(true);
                return boldFont;
                }
            case Qt::ForegroundRole:
                if (col == m_selected_column_index)
                {
                    return QBrush(Qt::white);
                }
                else
                {
                    return QBrush(Qt::gray);
                }

            case Qt::BackgroundRole:
                if (col == m_selected_column_index)
                {
                    return QBrush(QColor(0xdd2222));
                }
                else
                {
                    return QBrush(QColor(0x111122));
                }

            case Qt::TextAlignmentRole:
                return Qt::AlignHCenter + Qt::AlignVCenter;
            default:
                break;
        }
    }
    return QVariant();
}
