#include "progressbardelegate.h"
#include <QPainter>


//==================================================================
ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QAbstractItemDelegate(parent),
      m_colour_total(Qt::darkGreen),
      m_colour_upper(Qt::yellow),
      m_colour_lower(Qt::magenta),
      m_colour_text(Qt::black)
{

}

//==================================================================
QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
    return QSize(option.rect.width(), option.rect.height());
}

//==================================================================
void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QString mtc = index.data(Qt::DisplayRole).toString();
    if (mtc.isEmpty() == false)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        const int max_width = option.rect.width();
        const int height = option.rect.height();
        double pct = index.data(Qt::BackgroundRole).toDouble();
        int bar_width = static_cast<int>(pct*max_width);
        painter->fillRect(QRectF(option.rect.x(),option.rect.y(),bar_width,height),m_colour_total);

        double pct2 = index.data(Qt::UserRole).toDouble();
        bar_width = static_cast<int>(pct2*bar_width);
        painter->fillRect(QRectF(option.rect.x(),option.rect.y(),bar_width,height),m_colour_upper);

        double pct3 = index.data(Qt::ForegroundRole).toDouble();
        bar_width = static_cast<int>(pct3*bar_width);
        painter->fillRect(QRectF(option.rect.x(),option.rect.y(),bar_width,height),m_colour_lower);

        painter->setPen(m_colour_text);
        painter->setFont(option.font);

        painter->drawText(option.rect,Qt::AlignCenter,mtc);
        painter->restore();
    }
}

//==================================================================
void ProgressBarDelegate::applyDarkTheme(const QColor& total,
                                         const QColor& upper,
                                         const QColor& lower,
                                         const QColor& text)
{
    m_colour_text = text;
    m_colour_total = total;
    m_colour_lower = lower;
    m_colour_upper = upper;
}

//==================================================================
void ProgressBarDelegate::setColourText(const QColor& c)
{
    m_colour_text = c;
}

//==================================================================
void ProgressBarDelegate::setColourTotal(const QColor& c)
{
    m_colour_total = c;
}

//==================================================================
void ProgressBarDelegate::setColourUpper(const QColor& c)
{
    m_colour_upper = c;
}

//==================================================================
void ProgressBarDelegate::setColourLower(const QColor& c)
{
    m_colour_lower = c;
}
