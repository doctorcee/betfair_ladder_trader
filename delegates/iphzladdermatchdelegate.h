#ifndef IPHZLADDERMATCHDELEGATE_H
#define IPHZLADDERMATCHDELEGATE_H

#include <QAbstractItemDelegate>
#include <QColor>

class IPHZLadderMatchDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    IPHZLadderMatchDelegate(QObject* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const override;

    void setMatchColour(const QColor& c);

private:
    QColor m_match_colour;

};

#endif // IPHZLADDERMATCHDELEGATE_H
