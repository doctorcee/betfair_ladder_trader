#include "iphzladdermatchdelegate.h"

//==================================================================
IPHZLadderMatchDelegate::IPHZLadderMatchDelegate(QObject* parent)
    : QAbstractItemDelegate(parent),
      m_match_colour()
{

}

//==================================================================
QSize IPHZLadderMatchDelegate::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
    return QSize(option.rect.width(), option.rect.height());
}

//==================================================================
void IPHZLadderMatchDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    /*
    QString mtc = index.data(Qt::DisplayRole).toString();
    if (mtc.isEmpty() == false)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const int max_width = option.rect.width();
        const int height = option.rect.height();
        double pct = index.data(Qt::BackgroundRole).toDouble();
        int bar_width = static_cast<int>(pct*max_width);
        painter->fillRect(QRectF(option.rect.x(),option.rect.y(),bar_width,height),m_colour_upper);

        double pct2 = index.data(Qt::UserRole).toDouble();
        bar_width = static_cast<int>(pct2*bar_width);
        painter->fillRect(QRectF(option.rect.x(),option.rect.y(),bar_width,height),m_colour_lower);

        painter->setPen(m_colour_text);
        painter->setFont(option.font);

        painter->drawText(option.rect,Qt::AlignCenter,mtc);
        painter->restore();
    }
    */
}

//==================================================================
void IPHZLadderMatchDelegate::setMatchColour(const QColor &c)
{
    m_match_colour = c;
}

