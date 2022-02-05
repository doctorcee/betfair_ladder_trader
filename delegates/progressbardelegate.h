#ifndef PROGRESSBARDELEGATE_H
#define PROGRESSBARDELEGATE_H

#include <QAbstractItemDelegate>
#include <QColor>


class ProgressBarDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    ProgressBarDelegate(QObject* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const override;

    void setColourTotal(const QColor& c);
    void setColourUpper(const QColor& c);
    void setColourLower(const QColor& c);
    void setColourText(const QColor& c);

    void applyDarkTheme(const QColor& total,
                         const QColor& upper,
                         const QColor& lower,
                         const QColor& text);

private:
    QColor m_colour_total;
    QColor m_colour_upper;
    QColor m_colour_lower;
    QColor m_colour_text;

};

#endif // PROGRESSBARDELEGATE_H
