#ifndef STAKINGBOXMODEL_H
#define STAKINGBOXMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>

class StakingBoxModel : public QAbstractTableModel
{
public:

    StakingBoxModel(QObject *parent,
                    int num_stakes,
                    const std::uint16_t& disptheme);
    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int getRowCount() const {return m_row_count;}
    int getColCount() const {return m_col_count;}

    void updateAllData();

    void changeBaseStake(const std::uint32_t& new_stake);
    void setSelectedColumn(const int& col);
    std::uint32_t getSelectedStake() const {return m_selected_stake;}

    void setDisplayTheme(const std::uint16_t& disptheme) {m_display_theme = disptheme;}
    std::uint16_t getDisplayTheme() const {return m_display_theme;}

private:

    const int m_row_count;
    const int m_col_count;
    int m_selected_column_index;

    std::uint32_t m_base_stake;
    std::uint32_t m_selected_stake;
    std::uint16_t m_display_theme;

};

#endif // STAKINGBOXMODEL_H
